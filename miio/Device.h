//
// Created by lsk on 12/4/23.
//

#pragma once


#include <nlohmann/json.hpp>

using json = nlohmann::json;

#include "aes.h"

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
    struct sockaddr_in* addr;
    int inited_ = 0;
    unsigned char token_[16];
    AES_ctx ctx_;
    uint8_t iv_[16];
    net_port udp_client_;
    unsigned char did_[4];
    uint32_t root_stamp_;
    inline int UdpSendSync(void* send_buf, uint32_t send_buf_size, void* recv_buf, uint32_t recv_buf_size)
    {
        udp_client_.udp_write((char*)send_buf, send_buf_size);

        return udp_client_.udp_listen((char*)recv_buf, recv_buf_size);
    }
    static void Token2AesCtx(const unsigned char* token, AES_ctx* ctx, uint8_t* iv);
    static void UnpackHandshakeMsg(const unsigned char* msg_buf, Msg* msg);
    static void UnpackMsgHeader(const uint8_t* msg_buf, Msg* msg);
    void DecryptPayloads(uint8_t* payloads, uint16_t payloads_len);
    void EncryptPayloads(uint8_t* payloads, uint16_t payloads_len);
    static void PackMsgHeader(const Msg* msg, uint8_t* msg_buf);
    static void FillMd5Checksum(unsigned char* msg_buf, uint32_t packet_length);
    static void MakePayloads16Multiple(std::string& payloads_send);
    static void GetPayloadsActualLength(uint8_t* payloads, uint16_t* payloads_len);
    uint32_t packet_id_ = 0;
    inline int GetPacketIdPlus1()
    {
        return ++packet_id_;
    }
};
