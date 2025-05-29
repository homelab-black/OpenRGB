//
// Created by lsk on 12/4/23.
//

#pragma once


#include <mbedtls/aes.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "net_port.h"

class Device {
public:
    Device(const char* ip, const char* token);
    json send(json &command);

private:
    struct Msg
    {
        uint16_t length;
        unsigned char did[4];
        uint32_t stamp;
        unsigned char md5_checksum[16];
    };

    unsigned char token_[16];
    unsigned char key_[16];
    unsigned char iv_[16];

    net_port udp_client_;

    unsigned char did_[4];
    uint32_t root_stamp_;
    uint32_t packet_id_;

    int IncPacketId();
    int UdpSendSync(void* send_buf, uint32_t send_buf_size, void* recv_buf, uint32_t recv_buf_size);
    void Token2AesCtx();
    void DecryptPayloads(uint8_t* payloads, uint16_t payloads_len);
    void EncryptPayloads(uint8_t* payloads, uint16_t payloads_len);

    static void UnpackHandshakeMsg(const unsigned char* msg_buf, Msg* msg);
    static void UnpackMsgHeader(const uint8_t* msg_buf, Msg* msg);
    static void PackMsgHeader(const Msg* msg, uint8_t* msg_buf);
    static void FillMd5Checksum(unsigned char* msg_buf, uint32_t packet_length);
    static void MakePayloads16Multiple(std::string& payloads_send);
    static void GetPayloadsActualLength(uint8_t* payloads, uint16_t* payloads_len);

};
