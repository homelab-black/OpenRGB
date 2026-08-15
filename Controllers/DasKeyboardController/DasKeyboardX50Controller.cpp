/* Das Keyboard X50 RGB controller - SPDX-License-Identifier: GPL-2.0-or-later */
#include "DasKeyboardX50Controller.h"
#include "LogManager.h"
#include <chrono>
#include <cstring>
#include <thread>

using namespace std::chrono_literals;

static const unsigned int X50_REPORT_SIZE = 64;
static const unsigned int X50_PAYLOAD_SIZE = 60;
static const unsigned int X50_CHANNEL_STRIDE = 144;
static const unsigned int X50_TRANSFER_TIMEOUT_MS = 1000;
/* Python test was validated at 5 FPS.  A complete frame has
   sixteen reports (0x0A + 0x09), so keep enough quiet time between frames
   to stay close to that proven update rate and avoid wedging the firmware. */
static const unsigned int X50_FRAME_COOLDOWN_MS = 165;
static const unsigned char X50_OUT_ENDPOINT = 0x03;

static const DasKeyboardX50Led x50_leds[] =
{
    {"Key: F5",4,0},{"Key: 6",5,0},{"Key: Y",6,0},{"Key: H",7,0},{"Key: N",8,0},{"Key: F6",9,0},{"Key: 7",10,0},
    {"Key: F3",13,0},{"Key: 4",14,0},{"Key: R",15,0},{"Key: G",16,0},{"Key: B",17,0},{"Key: F4",18,0},{"Key: 5",19,0},
    {"Key: T",20,0},{"Key: F2",22,0},{"Key: 3",23,0},{"Key: E",24,0},{"Key: D",25,0},{"Key: V",26,0},{"Key: Space",27,0},
    {"Key: C",28,0},{"Key: F",29,0},{"Key: F1",31,0},{"Key: 2",32,0},{"Key: W",33,0},{"Key: S",34,0},{"Key: Z",35,0},
    {"Key: Left Alt",36,0},{"Key: X",37,0},{"Key: 1",41,0},{"Key: Q",42,0},{"Key: A",43,0},{"Key: Left Windows",45,0},
    {"Key: Escape",49,0},{"Key: Back Tick",50,0},{"Key: Tab",51,0},{"Key: Caps Lock",52,0},{"Key: Left Shift",53,0},{"Key: Left Control",54,0},
    {"Key: F12",58,0},{"Key: Backslash",60,79},{"Key: Enter",62,0},{"Key: Print Screen",63,0},{"Key: Backspace",64,0},{"Key: Insert",65,0},
    {"Key: F10",67,0},{"Key: Equals",68,0},{"Key: Left Bracket",69,0},{"Key: Quote",70,0},{"Key: Right Control",71,0},{"Key: F11",72,0},
    {"Key: Right Bracket",73,0},{"Key: Right Shift",74,0},{"Key: Minus",76,0},{"Key: P",77,0},{"Key: Semicolon",78,0},{"Key: Forward Slash",80,0},
    {"Key: F9",81,0},{"Key: Menu",82,0},{"Key: Function",83,0},{"Key: 0",85,0},{"Key: I",86,0},{"Key: K",87,0},{"Key: L",88,0},
    {"Key: Comma",89,0},{"Key: Period",90,0},{"Key: O",91,0},{"Key: Right Alt",92,0},{"Key: F7",94,0},{"Key: 8",95,0},{"Key: U",96,0},
    {"Key: J",97,0},{"Key: M",98,0},{"Key: F8",99,0},{"Key: 9",100,0},{"Key: Scroll Lock",103,0},{"Key: Page Down",104,0},
    {"Key: Delete",105,0},{"Key: Num Lock",106,0},{"Key: End",107,0},{"Key: Pause/Break",108,0},{"Key: Home",109,0},{"Key: Page Up",110,0},
    {"Key: Number Pad 2",113,0},{"Key: Number Pad /",114,0},{"Key: Brightness",115,0},{"Key: Number Pad 5",116,0},{"Key: Number Pad 8",119,0},
    {"Key: Number Pad -",123,0},{"Key: Media Play/Pause",124,0},{"Key: Number Pad Enter",125,0},{"Key: Number Pad +",128,0},
    {"Key: Number Pad 3",131,0},{"Key: Number Pad *",132,0},{"Key: Media Next",133,0},{"Key: Number Pad 6",134,0},{"Key: Number Pad .",136,0},
    {"Key: Number Pad 9",137,0},{"Key: Up Arrow",139,0},{"Key: Number Pad 1",140,0},{"Key: Down Arrow",141,0},{"Key: Right Arrow",142,0},
    {"Key: Number Pad 4",143,0},{"Key: Left Arrow",144,0},{"Key: Number Pad 0",145,0},{"Key: Number Pad 7",146,0},
    {"Volume Ring 0-90",130,0},{"Volume Ring 90-180",121,0},{"Volume Ring 180-270",135,0},{"Volume Ring 270-360",122,0}
};

DasKeyboardX50Controller::DasKeyboardX50Controller(libusb_context* usb_context, libusb_device_handle* device_handle, const std::string& path)
    : context(usb_context), device(device_handle), location(path), update_pending(false), update_thread_running(true),
      device_healthy(true), failure_warning_logged(false)
{
    device_healthy = Initialize();
    update_thread = std::thread(&DasKeyboardX50Controller::UpdateThreadFunction, this);
}

DasKeyboardX50Controller::~DasKeyboardX50Controller()
{
    {
        std::lock_guard<std::mutex> lock(update_mutex);
        update_thread_running = false;
    }
    update_condition.notify_one();
    update_thread.join();

    if(device != nullptr)
    {
        libusb_release_interface(device, 2);
        libusb_close(device);
        device = nullptr;
    }

    if(context != nullptr)
    {
        libusb_exit(context);
        context = nullptr;
    }
}

std::string DasKeyboardX50Controller::GetLocationString() const { return("USB: " + location); }

std::string DasKeyboardX50Controller::GetSerialString() const
{
    libusb_device_descriptor descriptor;
    unsigned char serial[128] = {0};
    if(libusb_get_device_descriptor(libusb_get_device(device), &descriptor) != LIBUSB_SUCCESS || descriptor.iSerialNumber == 0) return("");
    int length = libusb_get_string_descriptor_ascii(device, descriptor.iSerialNumber, serial, sizeof(serial));
    return(length > 0 ? std::string(reinterpret_cast<char*>(serial), static_cast<unsigned int>(length)) : "");
}

const DasKeyboardX50Led* DasKeyboardX50Controller::LedTable() { return(x50_leds); }
unsigned int DasKeyboardX50Controller::LedCount() { return(sizeof(x50_leds) / sizeof(x50_leds[0])); }

bool DasKeyboardX50Controller::SendReport(unsigned char command, unsigned char page, const unsigned char* payload)
{
    unsigned char report[X50_REPORT_SIZE] = {0};
    report[0] = 0x07; report[1] = command; report[2] = 0x06; report[3] = page;
    if(payload != nullptr) memcpy(&report[4], payload, X50_PAYLOAD_SIZE);
    int transferred = 0;
    int result = libusb_bulk_transfer(device, X50_OUT_ENDPOINT, report, static_cast<int>(sizeof(report)), &transferred, X50_TRANSFER_TIMEOUT_MS);

    if(result != LIBUSB_SUCCESS || transferred != static_cast<int>(sizeof(report)))
    {
        LOG_ERROR("[DasKeyboardX50] USB write failed for command 0x%02X page %u: %s (%d/%u bytes)",
                  command, page, libusb_error_name(result), transferred, X50_REPORT_SIZE);

        if(result == LIBUSB_ERROR_PIPE)
        {
            int clear_result = libusb_clear_halt(device, X50_OUT_ENDPOINT);
            LOG_WARNING("[DasKeyboardX50] Endpoint 0x%02X halt recovery: %s", X50_OUT_ENDPOINT, libusb_error_name(clear_result));
        }

        return(false);
    }

    std::this_thread::sleep_for(2ms);
    return(true);
}

bool DasKeyboardX50Controller::SendZeroPages(unsigned char command, unsigned int pages)
{
    for(unsigned int page = 0; page < pages; page++)
    {
        if(!SendReport(command, page, nullptr)) return(false);
    }
    return(true);
}

bool DasKeyboardX50Controller::SendColorPages(unsigned char command, const std::vector<RGBColor>& colors)
{
    unsigned char payload[8][X50_PAYLOAD_SIZE] = {{0}};
    const unsigned char fixed[][2] = {{2,31},{2,32},{2,41},{2,50},{2,58},{2,59},{3,4},{3,7},{3,8},{3,16},{3,17},{3,19},{3,21},{3,26},{3,35},{3,39},{3,44},{3,53},{4,1},{4,2},{4,11},{4,12},{4,17},{4,18},{4,20},{4,26},{4,29},{4,38},{4,47}};
    for(const auto& item : fixed) payload[item[0]][item[1]] = 0xFF;

    unsigned int count = colors.size() < LedCount() ? colors.size() : LedCount();
    for(unsigned int led = 0; led < count; led++)
    {
        unsigned short positions[2] = {x50_leds[led].position, x50_leds[led].extra_position};
        unsigned char values[3] =
        {
            static_cast<unsigned char>(RGBGetRValue(colors[led])),
            static_cast<unsigned char>(RGBGetGValue(colors[led])),
            static_cast<unsigned char>(RGBGetBValue(colors[led]))
        };
        for(unsigned int pos_idx = 0; pos_idx < 2 && positions[pos_idx] != 0; pos_idx++)
        {
            unsigned int base = positions[pos_idx] - 4;
            for(unsigned int channel = 0; channel < 3; channel++)
            {
                unsigned int index = base + channel * X50_CHANNEL_STRIDE;
                payload[index / X50_PAYLOAD_SIZE][index % X50_PAYLOAD_SIZE] = values[channel];
            }
        }
    }
    for(unsigned int page = 0; page < 8; page++)
    {
        if(!SendReport(command, page, payload[page])) return(false);
    }
    return(true);
}

bool DasKeyboardX50Controller::Initialize()
{
    /* Detection must not alter the keyboard's current lighting.  The
       interface is already ready after it is claimed; defer all writes
       until OpenRGB is explicitly asked to update the colors. */
    return(true);
}

void DasKeyboardX50Controller::SendColors(const std::vector<RGBColor>& colors)
{
    {
        std::lock_guard<std::mutex> lock(update_mutex);
        pending_colors = colors;
        update_pending = true;
    }
    update_condition.notify_one();
}

void DasKeyboardX50Controller::UpdateThreadFunction()
{
    std::unique_lock<std::mutex> lock(update_mutex);

    while(update_thread_running)
    {
        update_condition.wait(lock, [this]{ return(update_pending || !update_thread_running); });

        if(!update_thread_running)
        {
            break;
        }

        std::vector<RGBColor> colors = pending_colors;
        update_pending = false;
        lock.unlock();

        if(!device_healthy)
        {
            if(!failure_warning_logged)
            {
                LOG_WARNING("[DasKeyboardX50] Further color updates are disabled because a USB transfer failed; reconnect or rescan the device to recover");
                failure_warning_logged = true;
            }
        }
        else
        {
            /* The protocol validated by x50_rgb_cli.py uses 0x0A for the
               key-press layer and 0x09 for the normal visible layer. */
            device_healthy = SendColorPages(0x0A, colors);

            if(device_healthy)
            {
                device_healthy = SendColorPages(0x09, colors);
            }

            if(device_healthy)
            {
                /* Keep the firmware-safe update rate off OpenRGB's device
                   lock.  Updates arriving here are coalesced to the newest
                   complete frame. */
                std::this_thread::sleep_for(std::chrono::milliseconds(X50_FRAME_COOLDOWN_MS));
            }
        }

        lock.lock();
    }
}
