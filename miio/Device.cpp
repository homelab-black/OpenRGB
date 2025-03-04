//
// Created by lsk on 12/4/23.
//

#include "Device.h"

#include "type_tools.h"

#include <mbedtls/md5.h>


Device::Device(const char *ip, const char *token) {

    if (!udp_client_.udp_client(ip, "54321"))
    {
        throw std::runtime_error("Failed to create UDP Client");
    }

    unsigned char token_buf[16];
    Hex2Ascii(token, token_buf);

    //set token
    memcpy(token_, token_buf, 16);
    //set aes ctx
    Token2AesCtx(token_, &ctx_, iv_);
    //handshake
    unsigned char handshake_buf[32] = { 0x21, 0x31, 0, 0x20, 0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    UdpSendSync(handshake_buf, 32, handshake_buf, 32);
    Msg msg;
    UnpackHandshakeMsg((unsigned char*)handshake_buf, &msg);
    //set did
    memcpy(did_, msg.did, 4);
    //set root_stamp
    time_t seconds;
    time(&seconds);
    root_stamp_ = (uint32_t)seconds - msg.stamp;
}

json Device::send(json &command) {
    // 1. Create Payload
    command["id"] = GetPacketIdPlus1();
    std::string payload = command.dump();

    // 2. encrypt and send
    MakePayloads16Multiple(payload);
    uint8_t* buf = new uint8_t[2000];
    uint16_t payloads_len = (uint16_t)payload.length();
    payload.copy((char*)&(buf[32]), payloads_len, 0);
    Msg msg;
    msg.length = uint16_t(32 + payloads_len);
    memcpy(msg.did, did_,4);
    time_t seconds;
    time(&seconds);
    msg.stamp = (uint32_t)seconds - root_stamp_;
    memcpy(msg.md5_checksum, token_, 16);
    PackMsgHeader(&msg, buf);
    EncryptPayloads(&(buf[32]), payloads_len);
    FillMd5Checksum(buf, msg.length);

    int ret = UdpSendSync(buf, msg.length, buf, 2000);
    if (ret < 0)
    {
        delete[] buf;
        throw std::runtime_error("Failed to send command: " + std::to_string(ret));
    }
    //do NOT need msg struct for received msg
    UnpackMsgHeader(buf, &msg);
    payloads_len = (uint16_t)(ret - 32);
    DecryptPayloads(&(buf[32]), payloads_len);
    GetPayloadsActualLength(&(buf[32]), &payloads_len);
    payload.clear();
    payload.append((char*)&(buf[32]), payloads_len);
    delete[] buf;

    return json::parse(payload);
}

void Device::Token2AesCtx(const unsigned char *token, AES_ctx *ctx_with_key, uint8_t *iv) {
    unsigned char key_byte[16];
    unsigned char iv_byte[16];

    mbedtls_md5_context md5_ctx;
    mbedtls_md5_init(&md5_ctx);

    // Generate key from token (first 16 bytes)
    mbedtls_md5_starts(&md5_ctx);
    mbedtls_md5_update(&md5_ctx, token, 16);
    mbedtls_md5_finish(&md5_ctx, key_byte);

    AES_init_ctx(ctx_with_key, key_byte);//AES_init_ctx only will use key_byte[0] to key_byte[15]

    mbedtls_md5_starts(&md5_ctx);
    mbedtls_md5_update(&md5_ctx, key_byte, 16);
    mbedtls_md5_update(&md5_ctx, token, 16);
    mbedtls_md5_finish(&md5_ctx, iv_byte);

    memcpy(iv, iv_byte, 16);
    //AES_init_ctx_iv(ctx, key_byte, iv_byte);

    mbedtls_md5_free(&md5_ctx);
}

void Device::UnpackHandshakeMsg(const unsigned char *msg_buf, Device::Msg *msg) {
    memcpy(&(msg->did), &(msg_buf[8]), 4);
    CharToUint32BigEndian(&(msg_buf[12]), &(msg->stamp));
}

void Device::UnpackMsgHeader(const uint8_t *msg_buf, Device::Msg *msg) {
    CharToUint16BigEndian(&(msg_buf[2]), &(msg->length));
    memcpy(&(msg->did), &(msg_buf[8]), 4);
    CharToUint32BigEndian(&(msg_buf[12]), &(msg->stamp));
    memcpy(&(msg->md5_checksum), &(msg_buf[16]), 16);
}

void Device::DecryptPayloads(uint8_t *payloads, uint16_t payloads_len) {
    AES_ctx_set_iv(&ctx_, iv_);
    AES_CBC_decrypt_buffer(&ctx_, payloads, payloads_len);
}

void Device::EncryptPayloads(uint8_t *payloads, uint16_t payloads_len) {
    AES_ctx_set_iv(&ctx_, iv_);
    AES_CBC_encrypt_buffer(&ctx_, payloads, payloads_len);
}

void Device::PackMsgHeader(const Device::Msg *msg, uint8_t *msg_buf) {
    msg_buf[0] = 0x21;
    msg_buf[1] = 0x31;
    Uint16ToCharBigEndian(&(msg->length), &(msg_buf[2]));
    memset(&(msg_buf[4]), 0, 4);
    memcpy(&(msg_buf[8]), &(msg->did), 4);
    Uint32ToCharBigEndian(&(msg->stamp), &(msg_buf[12]));
    memcpy(&(msg_buf[16]), &(msg->md5_checksum), 16);
}

void Device::FillMd5Checksum(unsigned char *msg_buf, uint32_t packet_length) {
    unsigned char md5_checksum[16];
    mbedtls_md5_context md5_ctx;

    // Initialize MD5 context
    mbedtls_md5_init(&md5_ctx);

    int ret = mbedtls_md5_starts(&md5_ctx);
    if (ret == 0) {
        ret = mbedtls_md5_update(&md5_ctx, msg_buf, packet_length);
    }
    if (ret == 0) {
        ret = mbedtls_md5_finish(&md5_ctx, md5_checksum);
    }

    if (ret == 0) {
        memcpy(&msg_buf[16], md5_checksum, 16);
    }

    mbedtls_md5_free(&md5_ctx);
}

void Device::MakePayloads16Multiple(std::string& payloads_send) {
    const char empty[15] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    uint32_t len = (uint32_t)payloads_send.length();
    int remain = len % 16;
    if (remain != 0)
    {
        remain = 16 - remain;
        payloads_send.append(empty,remain);
    }
}

void Device::GetPayloadsActualLength(uint8_t *payloads, uint16_t *payloads_len) {
    while (payloads[*payloads_len - 1] != 0x7d && payloads[*payloads_len - 1] != 0)
    {
        (*payloads_len)--;
    }
}
