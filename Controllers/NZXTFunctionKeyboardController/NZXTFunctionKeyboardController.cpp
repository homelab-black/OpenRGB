/*---------------------------------------------------------*\
| NZXTFunctionKeyboardController.cpp                        |
|                                                           |
|   Driver for NZXT_FUNCTION laptop per-key RGB keyboard    |
|   Protocol based on tuxedo-drivers ite_8291 module        |
|                                                           |
|   P4tchW0rk (aekira@proton.me)          June 2026         |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/
#include "LogManager.h"
#include "NZXTFunctionKeyboardController.h"
#include <cstring>
#include <algorithm>
#include <iostream>

NZXTFunctionKeyboardController::NZXTFunctionKeyboardController(const hid_device_info& info)
{
    libusb_init(&ctx);

    handle = libusb_open_device_with_vid_pid(ctx, 0x1e71, 0x2106);
    if (!handle) {
        libusb_exit(ctx);
        return;
    }

    // Auto-détache kernel driver
    libusb_set_auto_detach_kernel_driver(handle, 1);

    // Claim cette interface
    if (libusb_claim_interface(handle, NZXT_FUNCTION_KEYBOARD_RGB_INTERFACE_ID) < 0) {
        return;
    }
}

NZXTFunctionKeyboardController::~NZXTFunctionKeyboardController()
{
    libusb_release_interface(handle, NZXT_FUNCTION_KEYBOARD_RGB_INTERFACE_ID);
    libusb_close(handle);
    libusb_exit(ctx);
}

std::string NZXTFunctionKeyboardController::GetDeviceLocation()
{
    return("HID: TODO");
}

std::string NZXTFunctionKeyboardController::GetSerialString()
{
    return std::string();
    struct libusb_device_descriptor desc;
    libusb_device* device = libusb_get_device(handle);
    int result = libusb_get_device_descriptor(device, &desc);
    if(result < 0)
    {
        LOG_ERROR("[NZXT] libusb_get_device_descriptor failed: %s",
                  libusb_error_name(result));
        return std::string();
    }

    if(desc.iSerialNumber == 0)
    {
        LOG_WARNING("[NZXT] Pas de serial number disponible");
        return std::string();
    }

    unsigned char serial[256];
    result = libusb_get_string_descriptor_ascii(
        handle,
        desc.iSerialNumber,  // Index du serial
        serial,
        sizeof(serial)
        );

    if(result < 0)
    {
        LOG_ERROR("[NZXT] libusb_get_string_descriptor_ascii failed: %s",
                  libusb_error_name(result));
        return std::string();
    }

    return std::string(reinterpret_cast<char*>(serial));
}

std::string NZXTFunctionKeyboardController::GetFirmwareVersion()
{
    return std::string("todo");
}


void NZXTFunctionKeyboardController::setStaticColor(unsigned char red, unsigned char blue, unsigned char green) {

}

void print_map(const std::map<std::tuple<int, int, int>, std::vector<unsigned int>>& m) {
    for(const auto& [key, value] : m) {
        auto [a, b, c] = key;
        std::cout << "(" << a << ", " << b << ", " << c << ") -> [";

        bool first = true;
        for(int v : value) {
            if(!first) std::cout << ", ";
            std::cout << v;
            first = false;
        }
        std::cout << "]" << "; ";
    }
    std::cout << std::endl;
}

void NZXTFunctionKeyboardController::SendColors(std::map<std::tuple<int, int, int>, std::vector<unsigned int>> leds_states_by_color, unsigned char brightness) {


    print_map(leds_states_by_color);

    // Make packets payloads
    std::deque<unsigned char> leds_bytes;
    for(auto e : leds_states_by_color) {
        auto color = e.first;
        auto leds = e.second;
        std::vector<unsigned char> bitmap(16, 0); // Keys *16
        //unsigned char bitmap[16] = {0};
        for(auto led_id : leds) {
            int byte_index = led_id / 8;
            int bit_index = led_id % 8;
            bitmap[byte_index] |= (1 << bit_index);
        }

        leds_bytes.push_back(0x01); // Mode
        leds_bytes.insert(leds_bytes.end(), bitmap.begin(), bitmap.end()); // Key * 16
        leds_bytes.push_back(0x00); // Padding
        leds_bytes.push_back(0x00); // Padding
        leds_bytes.push_back(std::get<0>(color)); // Color
        leds_bytes.push_back(std::get<1>(color)); // Color
        leds_bytes.push_back(std::get<2>(color)); // Color
    }


    std::cout << "Led bytes: ";
    for(unsigned char byte : leds_bytes) {
        std::cout << std::hex << std::setfill('0') << std::setw(2)
        << static_cast<int>(byte);
    }
    std::cout << std::dec << std::endl;


    // Make full packets
    std::deque<std::vector<unsigned char>> packets;
    bool begin = true;
    while(leds_bytes.size()) {
        std::vector<unsigned char> packet(64, 0);
        size_t payload_offset = 5; // Different for the first packet

        if(begin) { // First packet is different
            begin = false;
            packet[0] = 0x43;
            packet[1] = 0xbd;
            packet[2] = 0x00; // Other packets to send count
            packet[3] = 0x30; // Padding
            packet[4] = 0x00; // Padding
        } else {
            packet[0] = 0x43;
            packet[1] = 0x3d;
            packet[2] = 0x00; // Other packets to send count
            payload_offset = 3;
        }
        size_t bytes_to_move = (std::min)(leds_bytes.size(), size_t(64-payload_offset));
        std::copy(leds_bytes.begin(), leds_bytes.begin() + bytes_to_move, packet.data() + payload_offset);
        leds_bytes.erase(leds_bytes.begin(), leds_bytes.begin() + bytes_to_move);

        packets.push_back(std::move(packet));
    }

    for(int i = 0; i < packets.size(); i++) { // Define packet counts
        packets[i][2] = packets.size() - (i+1);
    }

    // Send packets
    LOG_INFO("Sending %d paquets", packets.size());
    int i = 0;
    for(auto packet : packets) {

        std::cout << "Final packet " << i << " : ";
        i++;
        for(size_t i = 0; i < 64; ++i) {
            std::cout << std::hex << std::setfill('0') << std::setw(2)
            << static_cast<int>(packet[i]);
        }
        std::cout << std::dec << std::endl;

        send(packet.data());
        receive();
    }
    receive();


    unsigned char final[64] = {
        0x43, 0xb5, 0x00, 0x37, 0x00, 0x08, 0xff, 0x7f, 0xff, 0x5f, 0xff, 0x5f, 0xff, 0x7f, 0xff, 0x6d, 0x27, 0x7e, 0xfa, 0x47, 0xfe, 0x5f, 0x00, 0x00, 0x3d, 0x00, 0x02, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46, 0x00, 0x03, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
    send(final);
    receive();
    receive();

    ///
    //LOG_INFO("TEST");
    //LOG_INFO("SETTING COLORS: %d,%d,%d", color_data[0], color_data[1], color_data[2]);
}

void NZXTFunctionKeyboardController::send(unsigned char* rgb_data) {
    int transferred = 0;
    int res = libusb_interrupt_transfer(
        handle,
        NZXT_FUNCTION_KEYBOARD_RGB_SEND_ENDPOINT,
        rgb_data,
        NZXT_FUNCTION_KEYBOARD_DATA_SIZE,
        &transferred,
        1000
        );
    if(res != 0) {
        LOG_ERROR("libusb_interrupt_transfer error: %s", libusb_error_name(res));
    }
}

void NZXTFunctionKeyboardController::receive() {
    int transferred = 0;
    for (int i = 0; i < 1; i++) {
        unsigned char data_in[64] = {0};
        libusb_interrupt_transfer(
            handle,
            NZXT_FUNCTION_KEYBOARD_RGB_RECV_ENDPOINT,
            data_in,
            NZXT_FUNCTION_KEYBOARD_DATA_SIZE,
            &transferred,
            100
            );
    }
}
