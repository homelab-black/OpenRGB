/*---------------------------------------------------------*\
|  KingstonFuryDRAMController.cpp                           |
|                                                           |
|   Driver for Kingston Fury DDR4/5 RAM modules             |
|                                                           |
|   Geofrey Mon (geofbot)                       14 Jul 2024 |
|   Milan Cermak (krysmanta)                                |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-only                   |
\*---------------------------------------------------------*/

#include <cstring>
#include <vector>
#include "KingstonFuryDRAMController.h"
#include "RGBController.h"
#include "LogManager.h"

KingstonFuryDRAMController::KingstonFuryDRAMController(
    i2c_smbus_interface* bus, unsigned char base_addr, std::vector<int> slots)
{
    this->bus       = bus;
    this->base_addr = base_addr;
    this->slots     = slots;

    reg_cache.resize(slots.size());
}

std::string KingstonFuryDRAMController::GetDeviceLocation()
{
    std::string return_string(bus->device_name);
    return_string.append(", addresses [");
    for(std::size_t idx = 0; idx < slots.size(); idx++)
    {
        char addr[5];
        snprintf(addr, 5, "0x%02X", base_addr + slots[idx]);
        return_string.append(addr);
        if(idx < slots.size() - 1)
        {
            return_string.append(",");
        }
        else
        {
            return_string.append("]");
        }
    }
    return("I2C: " + return_string);
}

unsigned int KingstonFuryDRAMController::GetLEDCount()
{
    return(GetLEDPerDIMM() * (unsigned int)slots.size());
}

unsigned int KingstonFuryDRAMController::GetSlotCount()
{
    return((unsigned int)slots.size());
}

unsigned char KingstonFuryDRAMController::GetMode()
{
    unsigned char mode = 0;
    CachedRead(0, FURY_REG_MODE, &mode);
    return mode;
}

bool KingstonFuryDRAMController::SmbusRead(int slot_idx, unsigned char reg, unsigned char *val)
{
    if(val == NULL)
    {
        return false;
    }

    unsigned char device_addr = base_addr + slots[slot_idx];
    int res;

    for(int retries = 1; retries <= 5; retries++)
    {
        res = bus->i2c_smbus_read_word_data(device_addr, reg);
        if(res >= 0)
        {
            *val = (res >> 8) & 0xFF;
            LOG_DEBUG("[%s] %02X reading register &%02X=%02X; res=%02X",
                      FURY_CONTROLLER_NAME, device_addr, reg, *val, res);
            return true;
        }
        else
        {
            std::this_thread::sleep_for(3 * retries * FURY_DELAY);
        }
    }
    return false;
}

bool KingstonFuryDRAMController::SmbusWrite(int slot_idx, unsigned char reg, unsigned char val)
{
    unsigned char device_addr = base_addr + slots[slot_idx];
    int res;

    for(int retries = 1; retries <= 5; retries++)
    {
        res = bus->i2c_smbus_write_byte_data(device_addr,
                                             reg, val);
        LOG_DEBUG("[%s] %02X setting register &%02X=%02X; res=%02X",
                  FURY_CONTROLLER_NAME, device_addr, reg, val, res);
        if(res >= 0)
        {
            return true;
        }
        else
        {
            std::this_thread::sleep_for(3 * retries * FURY_DELAY);
        }
    }
    return false;
}

// returns whether a read was successful
bool KingstonFuryDRAMController::CachedRead(int slot_idx, unsigned char reg, unsigned char *val)
{
    if(val == NULL)
    {
        return false;
    }

    unsigned char device_addr = base_addr + slots[slot_idx];
    if(reg_cache[slot_idx].find(reg) == reg_cache[slot_idx].end())
    {
        if(SmbusRead(slot_idx, reg, val))
        {
            reg_cache[slot_idx][reg] = *val;
            return true;
        }
        LOG_ERROR("[%s] %02X failed to get register &%02X",
                  FURY_CONTROLLER_NAME, device_addr, reg);
        return false;
    }
    else
    {
        *val = reg_cache[slot_idx][reg];
        return true;
    }
}

// returns whether a write was actually performed
bool KingstonFuryDRAMController::CachedWrite(int slot_idx, unsigned char reg, unsigned char val)
{
    unsigned char device_addr = base_addr + slots[slot_idx];
    if(reg_cache[slot_idx].find(reg) == reg_cache[slot_idx].end() ||
       reg_cache[slot_idx][reg] != val)
    {
        if(SmbusWrite(slot_idx, reg, val))
        {
            reg_cache[slot_idx][reg] = val;
            return true;
        }
        LOG_ERROR("[%s] %02X failed to set register &%02X=%02X",
                  FURY_CONTROLLER_NAME, device_addr, reg, val);
        return false;
    }
    else
    {
        LOG_DEBUG("[%s] %02X register already set &%02X=%02X",
                  FURY_CONTROLLER_NAME, device_addr, reg, val);
        return false;
    }
}

void KingstonFuryDRAMController::SendPreamble(bool /*synchronize*/)
{
    SendBegin();

    for(std::size_t idx = 0; idx < slots.size(); idx++)
    {
        char written_index = 0;
#ifdef FURY_SYNC
        if(!synchronize)
        {
            // some modes set all indices to 0 so that the
            // individual sticks don't sync with each other
            written_index = 0;
        }
        else
        {
            /*--------------------------------------------------------------*\
            | The index tells physical location of the RAM slot              |
            | from the border to the CPU slot. On most motherboards,         |
            | the address relates to the slot location,                      |
            | but there are exceptions.                                      |
            | The official software writes the indices in decreasing order.  |
            |                                                                |
            | Hardware effects seem to support only up to 4 sticks.          |
            | So we give the first 4 and last 4 sticks separate numbering.   |
            \*--------------------------------------------------------------*/
            written_index = idx % 4;
        }
#endif
        LOG_DEBUG("[%s] %02X writing index %d",
                  FURY_CONTROLLER_NAME, base_addr + slots[idx],
                  written_index);
        SmbusWrite((int)idx, FURY_REG_INDEX, written_index);
    }
    // The RGB controller is a bit slow and requires delay;
    // however, we can delay once for all of the sticks instead of
    // delaying individually for each stick.
    std::this_thread::sleep_for(FURY_DELAY);

    SendApply();
}

void KingstonFuryDRAMController::SendBegin()
{
    // start with the outermost stick
    for(int idx = (int)slots.size() - 1; idx >= 0; idx--)
    {
        LOG_DEBUG("[%s] %02X beginning transaction",
                  FURY_CONTROLLER_NAME, base_addr + slots[idx]);
        SetTransactionMode(idx, FURY_BEGIN_TRNSFER);
    }
}

void KingstonFuryDRAMController::SendApply()
{
    for(int idx = 0; idx < (int)slots.size(); idx++)
    {
        LOG_DEBUG("[%s] %02X ending transaction",
                  FURY_CONTROLLER_NAME, base_addr + slots[idx]);
        SetTransactionMode(idx, FURY_END_TRNSFER);
    }
}

int KingstonFuryDRAMController::SetTransactionMode(int slot_idx, int val)
{
    bool readback = true;
    int res;
    int expected = (val << 8) | 0x5A;
    int tries = 0;
    unsigned char addr = base_addr + slots[slot_idx];
    while(1) { // TODO timeout
        if(!readback) {
            res = bus->i2c_smbus_write_byte_data(addr, 0x08, val);
            //printf("settrans(0x%02X, 0x%02X) write: 0x%08X\n", addr, val, res);
            if (res < 0) {
            }
            transactionmode_last_write = steady_clock::now();
            readback = (res >= 0);
        }
        if(readback) {
            res = bus->i2c_smbus_read_word_data(addr, 0x08);
            if (res == expected)
                break;
            //printf("settrans(0x%02X, 0x%02X) read: 0x%08X\n", addr, val, res);
            if ((res & 0xFFFF00FF) == 0x5A)
                readback = false;
        }
        tries += 1;
        //BusyWaitFor(100us);
    }
    //printf("settrans(0x%02X, 0x%02X) read: 0x%08X tries %d\n", addr, val, res, tries);
    return tries;
}

void KingstonFuryDRAMController::SetMode(unsigned char val)
{
    SetRegister(FURY_REG_MODE, val);
}

void KingstonFuryDRAMController::SetNumSlots()
{
    if(slots.size() <= 4)
    {
        SetRegister(FURY_REG_NUM_SLOTS, (unsigned char)slots.size());
    }
    else
    {
        // hardware effects seem to only support at most 4 slots;
        // if there are >= 4 slots, then essentially the first 4 slots
        // run their effects independent of the last 4 slots
        SetRegister(FURY_REG_NUM_SLOTS, 4);
    }
}

void KingstonFuryDRAMController::SetRegister(int reg, unsigned char val)
{
    bool write_occurred = false;
    for(std::size_t idx = 0; idx < slots.size(); idx++)
    {
        write_occurred = CachedWrite((int)idx, reg, val) || write_occurred;
    }
    if(write_occurred)
    {
        std::this_thread::sleep_for(FURY_DELAY);
    }
}

void KingstonFuryDRAMController::SetRegister(int reg, std::vector<unsigned char> vals)
{
    bool write_occurred = false;
    if(vals.size() < slots.size())
    {
        LOG_ERROR("[%s] vector of values has wrong size when setting register &%02X",
                  FURY_CONTROLLER_NAME, reg);
        return;
    }
    for(std::size_t idx = 0; idx < slots.size(); idx++)
    {
        write_occurred = CachedWrite((int)idx, reg, vals[idx]) || write_occurred;
    }
    if(write_occurred)
    {
        std::this_thread::sleep_for(FURY_DELAY);
    }
}

void KingstonFuryDRAMController::SetModeColors(std::vector<RGBColor> colors)
{
    if(colors.empty() || (colors.size() > FURY_MAX_MODE_COLORS))
    {
        return;
    }

    SetRegister(FURY_REG_NUM_COLORS, (unsigned char)colors.size());

    for(std::size_t idx = 0; idx < colors.size(); idx++)
    {
        RGBColor color      = colors[idx];
        unsigned char red   = RGBGetRValue(color);
        unsigned char green = RGBGetGValue(color);
        unsigned char blue  = RGBGetBValue(color);

        int red_idx         = FURY_REG_MODE_BASE_RED   + (int)idx * 3;
        int green_idx       = FURY_REG_MODE_BASE_GREEN + (int)idx * 3;
        int blue_idx        = FURY_REG_MODE_BASE_BLUE  + (int)idx * 3;

        SetRegister(red_idx, red);
        SetRegister(green_idx, green);
        SetRegister(blue_idx, blue);
    }
}

void KingstonFuryDRAMController::SetLEDColors(std::vector<RGBColor> colors)
{
    if(colors.size() != GetLEDCount())
    {
        return;
    }

    unsigned int led_per_dimm = GetLEDPerDIMM();
    for(unsigned int led_idx = 0; led_idx < led_per_dimm; led_idx++)
    {
        int red_register    = FURY_REG_BASE_RED   + 3 * led_idx;
        int green_register  = FURY_REG_BASE_GREEN + 3 * led_idx;
        int blue_register   = FURY_REG_BASE_BLUE  + 3 * led_idx;

        std::vector<unsigned char> reds, greens, blues;
        for(std::size_t slot_idx = 0; slot_idx < GetSlotCount(); slot_idx++)
        {
            RGBColor color      = colors[slot_idx * led_per_dimm + led_idx];
            unsigned char red   = RGBGetRValue(color);
            unsigned char green = RGBGetGValue(color);
            unsigned char blue  = RGBGetBValue(color);

            reds.push_back(red);
            greens.push_back(green);
            blues.push_back(blue);
        }

        SetRegister(red_register, reds);
        SetRegister(blue_register, blues);
        SetRegister(green_register, greens);
    }
}

unsigned int KingstonFuryDRAMController::GetLEDPerDIMM()
{
    if(base_addr == FURY_BASE_ADDR_DDR4)
    {
        return(FURY_LEDS_PER_DIMM_DDR4);
    }
    return(FURY_LEDS_PER_DIMM_DDR5);
}

uint8_t leds[2][36];
uint16_t (*leds16)[18] = (uint16_t(*)[18])leds;

void effect() {
    static int counter;
    uint8_t c1, c2;
    int i, j, k;
    for (i = 0; i < 36; i += 3) {
        c1 = ((i / 3) == (counter % 12)) ? 255 : 0;
        for (j = 0; j < 3; j++) {
            c2 = (j == (counter / 12)) ? c1 : 0;
            for (k = 0; k < 2; k++) {
                leds[k][i + j] = c2;
            }
        }
    }
    counter = (counter + 1) % 36;
}

void breathein() {
    static int counter;
    uint8_t c;
    c = (1 << counter) - 1;
    int i, k;
    for (i = 0; i < 36; i += 1) {
        for (k = 0; k < 2; k++) {
            leds[k][i] = c;
        }
    }
    counter = (counter + 1) % 8;
}

void KingstonFuryDRAMController::test()
{
    int ledindex;
    int idx;
    int res;
    int addr;
    steady_clock::time_point t0;
    int w = 1;
    while (1) {
        //BusyWaitFor(300us);
        //settrans(0x63, 0x53);
        //t0 = Now();
        //BusyWaitFor(300us);
        //settrans(0x61, 0x53);
        //BusyWaitFor(300us);
        //w += 1;
        SendBegin();
        t0 = steady_clock::now();
        if(w > 10) {
            w = 1;
        }
        else {
            for (ledindex = 0; ledindex < 36; ledindex += 2) {
                for (idx = 0; idx < 2; idx++) {
                    addr = 0x61 + 2 * idx;
                    res = bus->i2c_smbus_write_word_data(addr, 0x50 + ledindex, leds16[idx][ledindex / 2]);
                    //res = bus->i2c_smbus_write_byte_data(addr, 0x50 + ledindex, leds[idx][ledindex]);

                    if (res < 0) {
                        printf("res 0x%08x\n", res);
                    }
                }
            }
        }

        //BusyWaitFor(300us);
        //settrans(0x61, 0x44);
        //BusyWaitFor(300us);
        //settrans(0x63, 0x44);
        SendApply();
        LOG_DEBUG("trans took %lld ns", (steady_clock::now() - t0).count());
        //BusyWaitFor(300us);

        //effect();
        breathein();
        std::this_thread::sleep_until(t0 + 30ms);

        //BusyWaitFor(3ms);

        int cnt = 0;
        while (1) {
            //std::this_thread::sleep_for(1ms);
            res = bus->i2c_smbus_read_word_data(0x63, 0x08);
            //printf("after endtrans res 0x%08X\n", res);
            if (res < 0)
                cnt += 1;
            if (cnt > 2)
                break;
        }
    }
}

void KingstonFuryDRAMController::testpreamble()
{
    SendBegin();
    bus->i2c_smbus_write_byte_data(0x63, 0x0B, 0x00); // FURY_REG_INDEX
    bus->i2c_smbus_write_byte_data(0x63, 0x09, 0x10);
    bus->i2c_smbus_write_byte_data(0x63, 0x20, 0x50);
    bus->i2c_smbus_write_byte_data(0x63, 0x27, 0x02);
    bus->i2c_smbus_write_byte_data(0x61, 0x0B, 0x01); // FURY_REG_INDEX
    bus->i2c_smbus_write_byte_data(0x61, 0x09, 0x10);
    bus->i2c_smbus_write_byte_data(0x61, 0x20, 0x50);
    bus->i2c_smbus_write_byte_data(0x61, 0x27, 0x02);
    SendApply();
    std::this_thread::sleep_for(20ms);
    SendBegin();

    //WriteIoPortByte(SMBusTiming, SMBusTimingFromFreq(200000));
    //WriteIoPortByte(SMBusTiming, 0xB0);

    //std::this_thread::sleep_for(1ms);
    //res = ReadIoPortByte(SMBusTiming);
    //printf("Timing 0x%02X\n", res);

    steady_clock::time_point t0, t1;
    for (int i = 0x50; i < 0x74; i += 1) {
        t0 = steady_clock::now();
        bus->i2c_smbus_write_byte_data(0x61, i, 0);
        t1 = steady_clock::now();
        //printf("dt %lld\n", (t1 - t0).count());
        bus->i2c_smbus_write_byte_data(0x63, i, 0);
    }
    SendApply();
    std::this_thread::sleep_for(40ms);
}
