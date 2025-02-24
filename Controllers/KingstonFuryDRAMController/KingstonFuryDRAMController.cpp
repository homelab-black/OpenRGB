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
//#include "RGBController.h"
#include "LogManager.h"

static inline time_point now()
{
    return std::chrono::steady_clock::now();
}

static inline double to_ms(duration dt)
{
    return std::chrono::nanoseconds(dt).count() / 1000000.0;
}

static inline void BusyWaitUntil(time_point tp)
{
    while(now() < tp)
    {
    }
}

static inline void BusyWaitFor(duration dt)
{
    BusyWaitUntil(now() + dt);
}

KingstonFuryDRAMController::KingstonFuryDRAMController(
    i2c_smbus_interface* bus, unsigned char base_addr, std::vector<int> slots)
{
    this->bus       = bus;
    this->base_addr = base_addr;
    this->slots     = slots;

    reg_cache.resize(slots.size());
    led_color_regs.resize(slots.size());

    time_point t0 = now();
    last_begin_trans_start = t0;
    last_begin_trans_master = t0;
    last_begin_trans_end = t0;
    last_end_trans_start = t0;
    last_end_trans_master = t0;
    last_end_trans_end = t0;
    last_offline_start  = t0;
    last_offline_detect  = t0;
    last_offline_end  = t0;
}

std::string KingstonFuryDRAMController::GetDeviceLocation()
{
    std::string return_string(bus->device_name);
    return_string.append(", addresses [");
    for(int idx = 0; idx < GetSlotCount(); idx++)
    {
        char addr[5];
        snprintf(addr, 5, "0x%02X", base_addr + slots[idx]);
        return_string.append(addr);
        if(idx < GetSlotCount() - 1)
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
    return(GetLEDPerDIMM() * (unsigned int)GetSlotCount());
}

int KingstonFuryDRAMController::GetSlotCount()
{
    return((int)slots.size());
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
            std::this_thread::sleep_for(3 * retries * FURY_DELAY_RETRY);
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
            std::this_thread::sleep_for(3 * retries * FURY_DELAY_RETRY);
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

    int last_slot = GetSlotCount() - 1;
    char written_index = 0;
    // start with the outermost stick
    for(int idx = last_slot; idx >= 0; idx--)
    {
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
        written_index += 1;
    }
    // The RGB controller is a bit slow and requires delay;
    // however, we can delay once for all of the sticks instead of
    // delaying individually for each stick.
    std::this_thread::sleep_for(FURY_DELAY_WRITE_REGISTER);

    SendApply();

    SendBegin();
    SetNumSlots();
    SendApply();
}

void KingstonFuryDRAMController::SendBegin()
{
    last_begin_trans_start = LogTimestamp("begin trans started", now());
    int last_slot = GetSlotCount() - 1;
    // start with the outermost stick
    for(int idx = last_slot; idx >= 0; idx--)
    {
        LOG_DEBUG("[%s] %02X beginning transaction",
                  FURY_CONTROLLER_NAME, base_addr + slots[idx]);
        SetTransactionMode(idx, FURY_BEGIN_TRNSFER);
        if(idx == last_slot)
        {
            last_offline_end = LogTimestamp("detected online", transactionmode_first_read);
            double dt = to_ms(transactionmode_last_write - last_begin_trans_master);
            if(dt < 200)
            {
                static int count = 0;
                static const int window = 100;
                period_rolling_average -= period_rolling_average / window;
                period_rolling_average += dt / window;
                LOG_DEBUG("[%s] -------------------------------------------------- PERIOD %f ms avg %f ms (%f Hz) count %d --------------------------------------------------",
                          FURY_CONTROLLER_NAME, dt, period_rolling_average, 1000 / period_rolling_average, ++count);
            }
            else
            {
                LOG_DEBUG("[%s] -------------------------------------------------- PERIOD %f ms (ignored for average) --------------------------------------------------",
                          FURY_CONTROLLER_NAME, dt);
            }
            last_begin_trans_master = LogTimestamp("begin trans went through on master", transactionmode_last_write);
        }
    }
    last_begin_trans_end = LogTimestamp("begin trans finished", now());
}

void KingstonFuryDRAMController::SendApply()
{
    last_end_trans_start = LogTimestamp("end trans started", now());
    int last_slot = GetSlotCount() - 1;
    for(int idx = 0; idx <= last_slot; idx++)
    {
        LOG_DEBUG("[%s] %02X ending transaction",
                  FURY_CONTROLLER_NAME, base_addr + slots[idx]);
        SetTransactionMode(idx, FURY_END_TRNSFER);
    }
    last_end_trans_master = LogTimestamp("end trans went through on master", transactionmode_last_write);
    last_end_trans_end = LogTimestamp("end trans verified on master", now());
    std::this_thread::sleep_until(last_begin_trans_master + 30ms);

    int cnt = 0;
    int tries = 0;
    int nofailcounter = 0;
    bool nofailseen = true;
    int expected = (FURY_END_TRNSFER << 8) | FURY_READ_SUCCESS;
    int res;
    unsigned char addr = base_addr + slots[last_slot];
    last_offline_start = LogTimestamp("started looking for offline", now());
    while(1)
    {
        res = bus->i2c_smbus_read_word_data(addr, 0x08);
        LOG_DEBUG("i2c_smbus_read_word_data(0x%02x, 0x%02X):0x%08X", addr, 8, res);
        if(nofailseen)
        {
            if(res == expected)
            {
                nofailcounter += 1;
            }
            else
            {
                nofailseen = false;
                last_offline_detect = LogTimestamp("detected offline", now());
            }
        }
        if(res < 0)
            cnt += 1;
        else
            cnt = 0;
        tries += 1;
        if(cnt > 2)
            break;
        //std::this_thread::sleep_for(1ms);
    }
    LOG_DEBUG("[%s] %02X detected offline, %d reads before first fail, %d tries total",
          FURY_CONTROLLER_NAME, addr, nofailcounter, tries);
}

int KingstonFuryDRAMController::SetTransactionMode(int slot_idx, int val)
{
    bool readback = true;
    bool no_successful_read = true;
    int res;
    int expected = (val << 8) | FURY_READ_SUCCESS;
    int tries = 0;
    unsigned char addr = base_addr + slots[slot_idx];
    while(1) { // TODO timeout
        if(!readback) {
            res = bus->i2c_smbus_write_byte_data(addr, 0x08, val);
            if (res >= 0) {
                transactionmode_last_write = now();
                readback = true;
            }
            // not waiting here can overwrite the registers used for detection when SMBUS runs faster than 100 kHz
            BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
        }
        if(readback) {
            res = bus->i2c_smbus_read_word_data(addr, 0x08);
            if((res & 0xFFFF00FF) == FURY_READ_SUCCESS)
            {
                if(no_successful_read)
                {
                    transactionmode_first_read = now();
                    no_successful_read = false;
                }
                readback = false;
                if((res == expected) && (tries > 0))
                {
                    break;
                }
            }
        }
        tries += 1;
    }
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
        SetRegister(FURY_REG_NUM_SLOTS, GetSlotCount());
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
    for(int idx = 0; idx < GetSlotCount(); idx++)
    {
        write_occurred = CachedWrite(idx, reg, val) || write_occurred;
    }
    if(write_occurred)
    {
        std::this_thread::sleep_for(FURY_DELAY_WRITE_REGISTER);
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
    for(int idx = 0; idx < GetSlotCount(); idx++)
    {
        write_occurred = CachedWrite(idx, reg, vals[idx]) || write_occurred;
    }
    if(write_occurred)
    {
        std::this_thread::sleep_for(FURY_DELAY_WRITE_REGISTER);
    }
}

void KingstonFuryDRAMController::SetModeColors(std::vector<RGBColor> colors)
{
    if(colors.empty() || (colors.size() > FURY_MAX_MODE_COLORS))
    {
        return;
    }

    SetRegister(FURY_REG_NUM_COLORS, (unsigned char)colors.size());

    for(int idx = 0; idx < GetSlotCount(); idx++)
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

    // pack rgb values for each stick
    for(int slot_idx = 0; slot_idx < GetSlotCount(); slot_idx++)
    {
        unsigned char *color_ptr_8 = &led_color_regs[slot_idx].bytes[0];
        for(unsigned int led_idx = 0; led_idx < led_per_dimm; led_idx += 1)
        {
            RGBColor color = colors[slot_idx * led_per_dimm + led_idx];
            *color_ptr_8++ = RGBGetRValue(color);
            *color_ptr_8++ = RGBGetGValue(color);
            *color_ptr_8++ = RGBGetBValue(color);
        }
    }

    // write two bytes at once, interleave across sticks
    for(unsigned int reg_idx = 0; reg_idx < led_per_dimm * 3; reg_idx += 2)
    {
        time_point min_wait = now() + FURY_DELAY_WRITE_REGISTER;
        for(int slot_idx = 0; slot_idx < GetSlotCount(); slot_idx++)
        {
            unsigned char addr = base_addr + slots[slot_idx];
            uint16_t val = led_color_regs[slot_idx].words[reg_idx / 2];
            int res = bus->i2c_smbus_write_word_data(addr, FURY_REG_BASE_RED + reg_idx, val);
            if(res)
            {
                LOG_DEBUG("i2c_smbus_write_word_data(0x%02x, 0x%02X, 0x%04X):0x%08X", addr, FURY_REG_BASE_RED + reg_idx, val, res);
            }
        }
        BusyWaitUntil(min_wait);
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

time_point KingstonFuryDRAMController::LogTimestamp(const char *txt, time_point tp)
{
    LOG_DEBUG("[%s] %s event "
              "%f ms after begin trans started, %f ms after detected online, "
              "%f ms after begin trans went through on master, %f ms after begin trans finished, "
              "%f ms after end trans started, %f ms after end trans went through on master, %f ms after end trans verified on master, "
              "%f ms after started looking for offline, %f ms after detected offline",
              FURY_CONTROLLER_NAME, txt,
              to_ms(tp - last_begin_trans_start), to_ms(tp - last_offline_end),
              to_ms(tp - last_begin_trans_master), to_ms(tp - last_begin_trans_end),
              to_ms(tp - last_end_trans_start), to_ms(tp - last_end_trans_master), to_ms(tp - last_end_trans_end),
              to_ms(tp - last_offline_start), to_ms(tp - last_offline_detect)
              );
    return tp;
}

void KingstonFuryDRAMController::testpreamble()
{
    int res;
    static bool firstrun = true;
    SendBegin();
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    if(firstrun)
    {
        LOG_DEBUG("[%s] register contents on startup follows", FURY_CONTROLLER_NAME);
        for(int slot_idx = 0; slot_idx < GetSlotCount(); slot_idx++)
        {
            unsigned char device_addr = base_addr + slots[slot_idx];
            for(int reg = 0; reg < 0x80; reg++)
            {
                res = bus->i2c_smbus_read_word_data(device_addr, reg);
                LOG_DEBUG("[%s] %02X reading register &%02X=%02X; res=%08X",
                          FURY_CONTROLLER_NAME, device_addr, reg, (res >> 8) & 0xFF, res);
            }
        }
    }
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    bus->i2c_smbus_write_byte_data(0x63, 0x0B, 0x00); // FURY_REG_INDEX
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    bus->i2c_smbus_write_byte_data(0x63, 0x09, 0x10);
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    bus->i2c_smbus_write_byte_data(0x63, 0x20, 0x50);
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    bus->i2c_smbus_write_byte_data(0x63, 0x27, 0x02);
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    bus->i2c_smbus_write_byte_data(0x61, 0x0B, 0x01); // FURY_REG_INDEX
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    bus->i2c_smbus_write_byte_data(0x61, 0x09, 0x10);
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    bus->i2c_smbus_write_byte_data(0x61, 0x20, 0x50);
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    bus->i2c_smbus_write_byte_data(0x61, 0x27, 0x02);
    BusyWaitFor(FURY_DELAY_WRITE_REGISTER);
    SendApply();
    std::this_thread::sleep_for(100ms);
}
