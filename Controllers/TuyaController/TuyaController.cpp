/*---------------------------------------------------------*\
| TuyaController.cpp                                        |
|                                                             |
|   Driver for Tuya-based WiFi RGB lights (local LAN         |
|   protocol version 3.3)                                    |
|                                                             |
|   Packet layout (all integers big-endian):                 |
|     u32 prefix (0x000055AA) | u32 seqno | u32 cmd |         |
|     u32 length | payload[length-8] | u32 crc32 |            |
|     u32 suffix (0x0000AA55)                                 |
|                                                              |
|   "length" counts payload + crc32 + suffix.                 |
|   crc32 covers everything from the prefix through the       |
|   last payload byte.                                        |
|                                                              |
|   The payload is the device's JSON command, AES-128-ECB     |
|   encrypted (PKCS7 padding) with the raw 16 byte local      |
|   key. CONTROL commands additionally get the 15 byte        |
|   plaintext string "3.3" + 12 null bytes prepended in       |
|   front of the ciphertext; DP_QUERY does not.                |
|                                                              |
|   Verified against tinytuya (github.com/jasonacox/tinytuya) |
|   against a real Tuya-protocol WiFi LED strip.               |
|                                                              |
|   This file is part of the OpenRGB project                  |
|   SPDX-License-Identifier: GPL-2.0-or-later                 |
\*---------------------------------------------------------*/

#include <chrono>
#include <cstring>
#include <nlohmann/json.hpp>
#include <mbedtls/aes.h>
#include "CRC.h"
#include "TuyaController.h"

using json = nlohmann::json;

enum
{
    TUYA_CMD_CONTROL   = 7,
    TUYA_CMD_DP_QUERY  = 10,
};

static const unsigned int TUYA_PREFIX = 0x000055AA;
static const unsigned int TUYA_SUFFIX = 0x0000AA55;

static void PushU32BE(std::vector<unsigned char>& buffer, unsigned int value)
{
    buffer.push_back((unsigned char)((value >> 24) & 0xFF));
    buffer.push_back((unsigned char)((value >> 16) & 0xFF));
    buffer.push_back((unsigned char)((value >>  8) & 0xFF));
    buffer.push_back((unsigned char)( value         & 0xFF));
}

static std::vector<unsigned char> TuyaAesEcbEncrypt(const std::string& key, const std::vector<unsigned char>& plain)
{
    std::vector<unsigned char> padded = plain;
    unsigned char pad_len = (unsigned char)(16 - (padded.size() % 16));
    padded.insert(padded.end(), pad_len, pad_len);

    mbedtls_aes_context aes;
    mbedtls_aes_init(&aes);
    mbedtls_aes_setkey_enc(&aes, (const unsigned char*)key.data(), 128);

    std::vector<unsigned char> encrypted(padded.size());
    for(std::size_t offset = 0; offset < padded.size(); offset += 16)
    {
        mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, &padded[offset], &encrypted[offset]);
    }

    mbedtls_aes_free(&aes);
    return encrypted;
}

TuyaController::TuyaController(std::string ip, std::string device_id, std::string local_key)
{
    ip_address      = ip;
    device_id       = device_id;
    local_key       = local_key;
    location        = "IP: " + ip_address;
    sequence_number = 0;

    port.tcp_client(ip_address.c_str(), "6668");
}

TuyaController::~TuyaController()
{
    if(port.connected)
    {
        port.tcp_close();
    }
}

std::string TuyaController::GetLocation()
{
    return(location);
}

std::string TuyaController::GetDeviceID()
{
    return(device_id);
}

bool TuyaController::SendPacket(unsigned int command, const std::string& json_payload, bool prepend_version_header)
{
    std::vector<unsigned char> plain(json_payload.begin(), json_payload.end());
    std::vector<unsigned char> encrypted = TuyaAesEcbEncrypt(local_key, plain);

    std::vector<unsigned char> payload;
    if(prepend_version_header)
    {
        const char version_header[] = "3.3";
        payload.insert(payload.end(), version_header, version_header + 3);
        payload.insert(payload.end(), 12, 0x00);
    }
    payload.insert(payload.end(), encrypted.begin(), encrypted.end());

    std::vector<unsigned char> packet;
    unsigned int length = (unsigned int)payload.size() + 8;

    PushU32BE(packet, TUYA_PREFIX);
    PushU32BE(packet, sequence_number++);
    PushU32BE(packet, command);
    PushU32BE(packet, length);
    packet.insert(packet.end(), payload.begin(), payload.end());

    unsigned int crc = CRC::Calculate(packet.data(), packet.size(), CRC::CRC_32());
    PushU32BE(packet, crc);
    PushU32BE(packet, TUYA_SUFFIX);

    if(!port.connected && !port.tcp_client_connect())
    {
        return false;
    }

    /*-------------------------------------------------------*\
    | Tuya devices usually ack within a few hundred ms; bound |
    | the read so a silent/misconfigured device cannot hang   |
    | the calling thread.                                     |
    \*-------------------------------------------------------*/
    port.set_receive_timeout(1, 0);

    port.tcp_write((char*)packet.data(), (int)packet.size());

    /*-----------------------------------------------------*\
    | Drain and discard the reply. This driver is send-only |
    | (Static color mode), so the ack does not need to be   |
    | decrypted, it just needs to be read off the socket    |
    | before it is closed.                                  |
    \*-----------------------------------------------------*/
    char receive_buffer[256];
    port.tcp_listen(receive_buffer, sizeof(receive_buffer));
    port.tcp_close();

    return true;
}

void TuyaController::SetColorHSV(unsigned int hue, unsigned int saturation, unsigned int value)
{
    char color_hex[13];
    std::snprintf(color_hex, sizeof(color_hex), "%04x%04x%04x", hue, saturation, value);

    json dps;
    dps["20"] = true;
    dps["21"] = "colour";
    dps["24"] = std::string(color_hex);

    json command;
    command["gwId"]  = device_id;
    command["devId"] = device_id;
    command["uid"]   = device_id;
    command["t"]     = std::to_string((long long)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    command["dps"]   = dps;

    SendPacket(TUYA_CMD_CONTROL, command.dump(), true);
}

void TuyaController::SetPower(bool on)
{
    json dps;
    dps["20"] = on;

    json command;
    command["gwId"]  = device_id;
    command["devId"] = device_id;
    command["uid"]   = device_id;
    command["t"]     = std::to_string((long long)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    command["dps"]   = dps;

    SendPacket(TUYA_CMD_CONTROL, command.dump(), true);
}
