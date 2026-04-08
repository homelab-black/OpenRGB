/*---------------------------------------------------------*\
| LogitechHIDPPController.cpp                               |
|                                                           |
|   Unified Logitech HID++ 2.0 controller implementation   |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <thread>
#include <chrono>
#include "LogitechHIDPPController.h"
#include "LogManager.h"

using namespace std::chrono_literals;

#define LOG_TAG "[LogitechHIDPP]"

LogitechHIDPPController::LogitechHIDPPController
    (
    hid_device*                 dev,
    const char*                 path,
    uint8_t                     device_index,
    bool                        wireless,
    std::shared_ptr<std::mutex> mutex_ptr
    )
{
    this->dev           = dev;
    this->location      = path;
    this->device_index  = device_index;
    this->wireless      = wireless;
    this->mutex         = mutex_ptr;
    this->initialized   = false;
    this->frame_counter = 0;

    caps = {};

    /*---------------------------------------------------------*\
    | Default to standard HID++ transport; DiscoverTransport()   |
    | may change this during Probe() if Centurion is detected.   |
    \*---------------------------------------------------------*/
    transport.type              = HIDPP_TRANSPORT_STANDARD;
    transport.report_id         = LOGITECH_LONG_MESSAGE;
    transport.addressed         = false;
    transport.device_address    = 0x00;
    transport.bridge_feat_idx   = 0;
    transport.sub_device_id     = 0;
}

LogitechHIDPPController::~LogitechHIDPPController()
{
    if(initialized)
    {
        Shutdown();
    }

    if(dev)
    {
        hid_close(dev);
    }
}

/*---------------------------------------------------------*\
| Transport-layer I/O                                       |
|                                                           |
| SendMessage/ReadMessage dispatch to the appropriate       |
| transport implementation based on transport.type.         |
| SendAndReceive is a convenience wrapper.                  |
\*---------------------------------------------------------*/

int LogitechHIDPPController::SendMessage
    (
    uint8_t         feat_idx,
    uint8_t         function,
    const uint8_t*  data,
    size_t          len
    )
{
    switch(transport.type)
    {
        case HIDPP_TRANSPORT_CENTURION:
            return SendCenturion(feat_idx, function, data, len);

        case HIDPP_TRANSPORT_STANDARD:
        default:
            return SendStandard(feat_idx, function, data, len);
    }
}

int LogitechHIDPPController::ReadMessage
    (
    uint8_t*    feat_idx_out,
    uint8_t*    function_out,
    uint8_t*    data_out,
    size_t      data_max,
    int         timeout_ms
    )
{
    switch(transport.type)
    {
        case HIDPP_TRANSPORT_CENTURION:
            return ReadCenturion(feat_idx_out, function_out, data_out, data_max, timeout_ms);

        case HIDPP_TRANSPORT_STANDARD:
        default:
            return ReadStandard(feat_idx_out, function_out, data_out, data_max, timeout_ms);
    }
}

int LogitechHIDPPController::SendAndReceive
    (
    uint8_t         feat_idx,
    uint8_t         function,
    const uint8_t*  send_data,
    size_t          send_len,
    uint8_t*        recv_data,
    size_t          recv_max
    )
{
    int result = SendMessage(feat_idx, function, send_data, send_len);

    if(result < 0)
    {
        LOG_DEBUG("%s SendMessage failed (result=%d) for feat=0x%02X func=0x%02X",
                  LOG_TAG, result, feat_idx, function);
        return result;
    }

    /*---------------------------------------------------------*\
    | Loop to discard unsolicited notifications (battery, etc.) |
    | until we get a response matching our request or timeout.  |
    \*---------------------------------------------------------*/
    for(int attempts = 0; attempts < 10; attempts++)
    {
        uint8_t resp_feat = 0;
        uint8_t resp_func = 0;

        result = ReadMessage(&resp_feat, &resp_func, recv_data, recv_max);

        if(result <= 0)
        {
            LOG_DEBUG("%s Read timeout/error (result=%d, attempt=%d) for feat=0x%02X func=0x%02X",
                      LOG_TAG, result, attempts, feat_idx, function);
            return result;
        }

        /*-----------------------------------------------------*\
        | Check for HID++ error response                        |
        \*-----------------------------------------------------*/
        if(resp_feat == 0xFF)
        {
            /*-------------------------------------------------*\
            | HID++ error: resp_func = feat_idx of failed req,  |
            | recv_data[0] = func|swid, recv_data[1] = error   |
            \*-------------------------------------------------*/
            LOG_DEBUG("%s HID++ error for feat=0x%02X func=0x%02X: "
                      "err_feat=0x%02X err_func=0x%02X err_code=0x%02X",
                      LOG_TAG, feat_idx, function,
                      resp_func,
                      recv_data ? recv_data[0] : 0,
                      recv_data ? recv_data[1] : 0);
            return -1;
        }

        /*-----------------------------------------------------*\
        | Check if this response matches our request             |
        \*-----------------------------------------------------*/
        if(resp_feat == feat_idx &&
           (resp_func & 0xF0) == (function & 0xF0))
        {
            return result;
        }

        /* Silently discard unsolicited notifications */
    }

    LOG_DEBUG("%s SendAndReceive: gave up after 10 attempts", LOG_TAG);
    return -1;
}

/*---------------------------------------------------------*\
| SendLong / ReadResponse — backward-compatible wrappers    |
| These dispatch through SendMessage/ReadMessage which      |
| handle transport framing (standard vs Centurion).         |
\*---------------------------------------------------------*/

int LogitechHIDPPController::SendLong
    (
    uint8_t         feat_idx,
    uint8_t         function,
    const uint8_t*  data,
    size_t          len
    )
{
    return SendMessage(feat_idx, function, data, len);
}

int LogitechHIDPPController::ReadResponse
    (
    blankFAPmessage&    response,
    uint8_t             expected_feat_idx,
    uint8_t             expected_function,
    int                 timeout_ms,
    int                 retries_left
    )
{
    uint8_t resp_feat = 0;
    uint8_t resp_func = 0;
    uint8_t resp_data[60] = {};

    int result = ReadMessage(&resp_feat, &resp_func, resp_data, sizeof(resp_data), timeout_ms);

    if(result <= 0)
    {
        return result;
    }

    /*---------------------------------------------------------*\
    | Reconstitute into blankFAPmessage for callers that use it |
    \*---------------------------------------------------------*/
    response.init();
    response.report_id       = LOGITECH_LONG_MESSAGE;
    response.device_index    = device_index;
    response.feature_index   = resp_feat;
    response.feature_command = resp_func;
    memcpy(response.data, resp_data, sizeof(response.data));

    /*---------------------------------------------------------*\
    | Check for error and matching — same as old ReadResponse    |
    \*---------------------------------------------------------*/
    if(resp_feat == 0xFF)
    {
        LOG_DEBUG("%s HID++ error: feat_idx=0x%02X func|swid=0x%02X error=0x%02X",
                  LOG_TAG, resp_func, resp_data[0], resp_data[1]);
        return -1;
    }

    if(resp_feat == expected_feat_idx &&
       (resp_func & 0xF0) == (expected_function & 0xF0))
    {
        return result;
    }

    if(retries_left <= 0)
    {
        LOG_DEBUG("%s ReadResponse: gave up after max retries", LOG_TAG);
        return -1;
    }

    /* Silently discard and retry */
    return ReadResponse(response, expected_feat_idx, expected_function, timeout_ms, retries_left - 1);
}

/*---------------------------------------------------------*\
| Standard HID++ transport (0xFF00 / 0xFF43)                |
| Report IDs 0x10 (7 bytes) / 0x11 (20 bytes)              |
\*---------------------------------------------------------*/

int LogitechHIDPPController::SendStandard
    (
    uint8_t         feat_idx,
    uint8_t         function,
    const uint8_t*  data,
    size_t          len
    )
{
    longFAPrequest request;
    request.init(device_index, feat_idx, function | HIDPP_SW_ID);

    if(data && len > 0)
    {
        size_t copy_len = (len > sizeof(request.data)) ? sizeof(request.data) : len;
        memcpy(request.data, data, copy_len);
    }

    int result;

    if(mutex)
    {
        std::lock_guard<std::mutex> lock(*mutex);
        result = hid_write(dev, request.buffer, request.size());
    }
    else
    {
        result = hid_write(dev, request.buffer, request.size());
    }

    return result;
}

int LogitechHIDPPController::ReadStandard
    (
    uint8_t*    feat_idx_out,
    uint8_t*    function_out,
    uint8_t*    data_out,
    size_t      data_max,
    int         timeout_ms
    )
{
    blankFAPmessage response;
    response.init();

    int result;

    if(mutex)
    {
        std::lock_guard<std::mutex> lock(*mutex);
        result = hid_read_timeout(dev, response.buffer, response.size(), timeout_ms);
    }
    else
    {
        result = hid_read_timeout(dev, response.buffer, response.size(), timeout_ms);
    }

    if(result <= 0)
    {
        return result;
    }

    if(feat_idx_out)
    {
        *feat_idx_out = response.feature_index;
    }

    if(function_out)
    {
        *function_out = response.feature_command;
    }

    if(data_out && data_max > 0)
    {
        size_t copy_len = (data_max > sizeof(response.data)) ? sizeof(response.data) : data_max;
        memcpy(data_out, response.data, copy_len);
    }

    return result;
}

/*---------------------------------------------------------*\
| Centurion transport (0xFFA0)                              |
|                                                           |
| Wire format per protocol doc:                             |
|   0x51 (direct): [reportId] [cplLen] [flags] [featIdx]    |
|                  [func|swid] [params...]                   |
|   0x50 (addressed): [reportId] [devAddr] [cplLen] [flags] |
|                     [featIdx] [func|swid] [params...]      |
|                                                           |
| For sub-device access, the parent CentPPBridge wraps      |
| sub-device messages:                                      |
|   params = [devId<<4|lenHi, lenLo, subCPL, subFeatIdx,    |
|             subFunc|swid, subParams...]                    |
|                                                           |
| TODO: Full implementation — currently stubbed.            |
\*---------------------------------------------------------*/

int LogitechHIDPPController::SendCenturion
    (
    uint8_t         feat_idx,
    uint8_t         function,
    const uint8_t*  data,
    size_t          len
    )
{
    uint8_t buf[64];
    memset(buf, 0, sizeof(buf));

    if(transport.bridge_feat_idx != 0)
    {
        /*-----------------------------------------------------*\
        | Sub-device message routed through CentPPBridge         |
        | Parent message: feat=bridge, func=sendFragment(0x10)   |
        | Payload: [devId<<4|lenHi, lenLo, subCPL=0x00,          |
        |           subFeatIdx, subFunc|swid, subParams...]       |
        \*-----------------------------------------------------*/
        uint8_t sub_msg_len = 3 + (uint8_t)len;  // subCPL + featIdx + func + data

        if(transport.addressed)
        {
            buf[0] = transport.report_id;
            buf[1] = transport.device_address;
            buf[2] = 5 + sub_msg_len;              // cplLen
            buf[3] = 0x00;                          // flags (single fragment)
            buf[4] = transport.bridge_feat_idx;
            buf[5] = 0x10 | HIDPP_SW_ID;            // sendFragment (func 1)
            buf[6] = (transport.sub_device_id << 4) | ((sub_msg_len >> 8) & 0x0F);
            buf[7] = sub_msg_len & 0xFF;
            buf[8] = 0x00;                          // sub-CPL (single fragment)
            buf[9] = feat_idx;
            buf[10] = function | HIDPP_SW_ID;

            if(data && len > 0)
            {
                memcpy(buf + 11, data, len);
            }
        }
        else
        {
            buf[0] = transport.report_id;
            buf[1] = 4 + sub_msg_len;              // cplLen
            buf[2] = 0x00;                          // flags
            buf[3] = transport.bridge_feat_idx;
            buf[4] = 0x10 | HIDPP_SW_ID;            // sendFragment (func 1)
            buf[5] = (transport.sub_device_id << 4) | ((sub_msg_len >> 8) & 0x0F);
            buf[6] = sub_msg_len & 0xFF;
            buf[7] = 0x00;                          // sub-CPL
            buf[8] = feat_idx;
            buf[9] = function | HIDPP_SW_ID;

            if(data && len > 0)
            {
                memcpy(buf + 10, data, len);
            }
        }
    }
    else
    {
        /*-----------------------------------------------------*\
        | Direct parent device message (no bridge)               |
        \*-----------------------------------------------------*/
        if(transport.addressed)
        {
            buf[0] = transport.report_id;
            buf[1] = transport.device_address;
            buf[2] = 3 + (uint8_t)len;             // cplLen
            buf[3] = 0x00;                          // flags
            buf[4] = feat_idx;
            buf[5] = function | HIDPP_SW_ID;

            if(data && len > 0)
            {
                memcpy(buf + 6, data, len);
            }
        }
        else
        {
            buf[0] = transport.report_id;
            buf[1] = 2 + (uint8_t)len;             // cplLen
            buf[2] = 0x00;                          // flags
            buf[3] = feat_idx;
            buf[4] = function | HIDPP_SW_ID;

            if(data && len > 0)
            {
                memcpy(buf + 5, data, len);
            }
        }
    }

    int result;

    if(mutex)
    {
        std::lock_guard<std::mutex> lock(*mutex);
        result = hid_write(dev, buf, 64);
    }
    else
    {
        result = hid_write(dev, buf, 64);
    }

    return result;
}

int LogitechHIDPPController::ReadCenturion
    (
    uint8_t*    feat_idx_out,
    uint8_t*    function_out,
    uint8_t*    data_out,
    size_t      data_max,
    int         timeout_ms
    )
{
    uint8_t buf[64];
    memset(buf, 0, sizeof(buf));

    int result;

    if(mutex)
    {
        std::lock_guard<std::mutex> lock(*mutex);
        result = hid_read_timeout(dev, buf, sizeof(buf), timeout_ms);
    }
    else
    {
        result = hid_read_timeout(dev, buf, sizeof(buf), timeout_ms);
    }

    if(result <= 0)
    {
        return result;
    }

    /*---------------------------------------------------------*\
    | Parse based on transport variant                          |
    \*---------------------------------------------------------*/
    int hdr_offset = transport.addressed ? 1 : 0;  // skip device address byte

    uint8_t cpl_len   = buf[1 + hdr_offset];
    // uint8_t cpl_flags = buf[2 + hdr_offset];     // for fragmentation support
    uint8_t resp_feat = buf[3 + hdr_offset];
    uint8_t resp_func = buf[4 + hdr_offset];

    if(transport.bridge_feat_idx != 0 && resp_feat == transport.bridge_feat_idx)
    {
        /*-----------------------------------------------------*\
        | CentPPBridge response — extract sub-device message     |
        | Skip: bridge header (devId|lenHi, lenLo, subCPL)       |
        \*-----------------------------------------------------*/
        int sub_offset = 5 + hdr_offset + 3;        // past bridge header
        resp_feat = buf[sub_offset];
        resp_func = buf[sub_offset + 1];

        if(feat_idx_out)   *feat_idx_out = resp_feat;
        if(function_out)   *function_out = resp_func;

        if(data_out && data_max > 0)
        {
            size_t avail = (size_t)(result - sub_offset - 2);
            size_t copy = (avail < data_max) ? avail : data_max;
            memcpy(data_out, buf + sub_offset + 2, copy);
        }
    }
    else
    {
        /*-----------------------------------------------------*\
        | Direct response                                        |
        \*-----------------------------------------------------*/
        if(feat_idx_out)   *feat_idx_out = resp_feat;
        if(function_out)   *function_out = resp_func;

        if(data_out && data_max > 0)
        {
            size_t avail = (size_t)(cpl_len > 2 ? cpl_len - 2 : 0);
            size_t copy = (avail < data_max) ? avail : data_max;
            memcpy(data_out, buf + 5 + hdr_offset, copy);
        }
    }

    (void)cpl_len;

    return result;
}

/*---------------------------------------------------------*\
| Feature Discovery                                         |
\*---------------------------------------------------------*/

uint8_t LogitechHIDPPController::GetFeatureIndex(uint16_t feature_page)
{
    uint8_t send_data[2];
    send_data[0] = (feature_page >> 8) & 0xFF;
    send_data[1] = feature_page & 0xFF;

    uint8_t recv_data[16] = {};
    int result = SendAndReceive(LOGITECH_HIDPP_PAGE_ROOT_IDX, FN_8071_GET_INFO,
                                send_data, 2, recv_data, sizeof(recv_data));

    if(result > 0)
    {
        uint8_t index = recv_data[0];
        LOG_DEBUG("%s Feature 0x%04X -> index 0x%02X", LOG_TAG, feature_page, index);
        return index;
    }

    LOG_DEBUG("%s Feature 0x%04X lookup failed (result=%d)", LOG_TAG, feature_page, result);
    return 0;
}

void LogitechHIDPPController::DiscoverDeviceName()
{
    uint8_t feat_idx = GetFeatureIndex(HIDPP_FEAT_DEVICE_NAME_TYPE);

    if(feat_idx == 0)
    {
        caps.device_name = "Logitech HID++ Device";
        return;
    }

    uint8_t recv[16] = {};
    int result = SendAndReceive(feat_idx, LOTITECH_CMD_DEVICE_NAME_TYPE_GET_COUNT,
                                nullptr, 0, recv, sizeof(recv));

    if(result <= 0)
    {
        caps.device_name = "Logitech HID++ Device";
        return;
    }

    unsigned int name_length = recv[0];
    caps.device_name.clear();

    for(unsigned int offset = 0; offset < name_length; offset += 16)
    {
        uint8_t send_data[1] = { (uint8_t)offset };
        result = SendAndReceive(feat_idx, LOGITECH_CMD_DEVICE_NAME_TYPE_GET_DEVICE_NAME,
                                send_data, 1, recv, sizeof(recv));

        if(result <= 0)
        {
            break;
        }

        unsigned int chunk_len = name_length - offset;
        if(chunk_len > 16)
        {
            chunk_len = 16;
        }

        caps.device_name.append((char*)recv, chunk_len);
    }

    LOG_VERBOSE("%s Device name: %s", LOG_TAG, caps.device_name.c_str());
}

void LogitechHIDPPController::DiscoverDeviceType()
{
    uint8_t feat_idx = GetFeatureIndex(HIDPP_FEAT_DEVICE_NAME_TYPE);

    if(feat_idx == 0)
    {
        caps.device_type = LOGITECH_DEVICE_TYPE_MOUSE;
        return;
    }

    uint8_t recv[16] = {};
    int result = SendAndReceive(feat_idx, LOGITECH_CMD_DEVICE_NAME_TYPE_GET_TYPE,
                                nullptr, 0, recv, sizeof(recv));

    if(result > 0)
    {
        caps.device_type = recv[0];
        LOG_VERBOSE("%s Device type: %d", LOG_TAG, caps.device_type);
    }
    else
    {
        caps.device_type = LOGITECH_DEVICE_TYPE_MOUSE;
    }
}

void LogitechHIDPPController::DiscoverTransport()
{
    /*---------------------------------------------------------*\
    | Detect Centurion transport by checking usage page.         |
    | Standard HID++ is the default; if we're on 0xFFA0 we      |
    | switch to Centurion framing and probe for CentPPBridge.    |
    |                                                           |
    | TODO: Implement full Centurion detection. For now we only  |
    | set the type flag — actual Centurion devices will fail     |
    | IRoot and be skipped until the transport is complete.      |
    \*---------------------------------------------------------*/
}

void LogitechHIDPPController::DiscoverRGBEffects()
{
    /*---------------------------------------------------------*\
    | Try 0x8071 first, fall back to 0x8070                     |
    \*---------------------------------------------------------*/
    caps.idx_rgb_effects = GetFeatureIndex(HIDPP_FEAT_RGB_EFFECTS);
    caps.rgb_feature_page = HIDPP_FEAT_RGB_EFFECTS;

    if(caps.idx_rgb_effects == 0)
    {
        caps.idx_rgb_effects = GetFeatureIndex(HIDPP_FEAT_COLOR_LED_EFFECTS);
        caps.rgb_feature_page = HIDPP_FEAT_COLOR_LED_EFFECTS;
    }

    if(caps.idx_rgb_effects == 0)
    {
        caps.has_zone_effects = false;
        return;
    }

    /*---------------------------------------------------------*\
    | GetInfo: discover cluster count                           |
    | 0x8071: data = [0xFF, 0xFF, 0x00]                        |
    | 0x8070: data = [] (empty)                                 |
    \*---------------------------------------------------------*/
    uint8_t data[3] = { 0xFF, 0xFF, 0x00 };
    size_t data_len = (caps.rgb_feature_page == HIDPP_FEAT_RGB_EFFECTS) ? 3 : 0;

    SendLong(caps.idx_rgb_effects, FN_8071_GET_INFO, data, data_len);

    blankFAPmessage response;
    int result = ReadResponse(response, caps.idx_rgb_effects, FN_8071_GET_INFO);

    if(result <= 0)
    {
        caps.has_zone_effects = false;
        return;
    }

    unsigned int cluster_count;

    if(caps.rgb_feature_page == HIDPP_FEAT_RGB_EFFECTS)
    {
        cluster_count = response.data[2];
    }
    else
    {
        cluster_count = response.data[0];
    }

    LOG_VERBOSE("%s Zone cluster count: %d", LOG_TAG, cluster_count);

    /*---------------------------------------------------------*\
    | GetRgbClusterInfo for each cluster                        |
    \*---------------------------------------------------------*/
    for(unsigned int i = 0; i < cluster_count; i++)
    {
        HIDPPZoneCluster cluster;
        cluster.index = i;

        if(caps.rgb_feature_page == HIDPP_FEAT_RGB_EFFECTS)
        {
            uint8_t query[2] = { (uint8_t)i, 0xFF };
            SendLong(caps.idx_rgb_effects, FN_8071_GET_INFO, query, 2);
        }
        else
        {
            uint8_t query[2] = { (uint8_t)i, 0x00 };
            SendLong(caps.idx_rgb_effects, LOGITECH_CMD_RGB_EFFECTS_GET_INFO, query, 2);
        }

        result = ReadResponse(response, caps.idx_rgb_effects,
                              (caps.rgb_feature_page == HIDPP_FEAT_RGB_EFFECTS) ? FN_8071_GET_INFO : LOGITECH_CMD_RGB_EFFECTS_GET_INFO);

        if(result <= 0)
        {
            continue;
        }

        if(caps.rgb_feature_page == HIDPP_FEAT_RGB_EFFECTS)
        {
            cluster.location     = (response.data[2] << 8) | response.data[3];
            cluster.effect_count = response.data[4];
        }
        else
        {
            cluster.location     = (response.data[1] << 8) | response.data[2];
            cluster.effect_count = response.data[3];
        }

        LOG_VERBOSE("%s Cluster %d: location=0x%04X effects=%d",
                    LOG_TAG, i, cluster.location, cluster.effect_count);

        /*-----------------------------------------------------*\
        | GetEffectInfo for each effect in this cluster          |
        \*-----------------------------------------------------*/
        for(unsigned int j = 0; j < cluster.effect_count; j++)
        {
            HIDPPEffect effect;
            effect.index = j;

            uint8_t eff_query[4] = { (uint8_t)i, (uint8_t)j, 0x00, 0x00 };
            SendLong(caps.idx_rgb_effects,
                     (caps.rgb_feature_page == HIDPP_FEAT_RGB_EFFECTS) ? FN_8071_GET_INFO : LOGITECH_FP8070_EFFECT_INFO,
                     eff_query, 4);

            result = ReadResponse(response, caps.idx_rgb_effects,
                                  (caps.rgb_feature_page == HIDPP_FEAT_RGB_EFFECTS) ? FN_8071_GET_INFO : LOGITECH_FP8070_EFFECT_INFO);

            if(result <= 0)
            {
                continue;
            }

            effect.effect_id     = (response.data[2] << 8) | response.data[3];
            effect.capabilities  = (response.data[4] << 8) | response.data[5];
            effect.default_period = (response.data[6] << 8) | response.data[7];

            LOG_DEBUG("%s   Effect %d: id=0x%04X caps=0x%04X period=%d",
                      LOG_TAG, j, effect.effect_id, effect.capabilities, effect.default_period);

            cluster.effects.push_back(effect);
        }

        caps.zone_clusters.push_back(cluster);
    }

    caps.has_zone_effects = !caps.zone_clusters.empty();
}

void LogitechHIDPPController::DiscoverPerKeyZones()
{
    /*---------------------------------------------------------*\
    | Try 0x8081 first, fall back to 0x8080                     |
    \*---------------------------------------------------------*/
    caps.idx_perkey_v2 = GetFeatureIndex(HIDPP_FEAT_PER_KEY_LIGHTING_V2);

    if(caps.idx_perkey_v2 == 0)
    {
        caps.idx_perkey_v1 = GetFeatureIndex(HIDPP_FEAT_PER_KEY_LIGHTING_V1);
    }

    uint8_t perkey_idx = (caps.idx_perkey_v2 != 0) ? caps.idx_perkey_v2 : caps.idx_perkey_v1;

    if(perkey_idx == 0)
    {
        caps.has_perkey = false;
        return;
    }

    /*---------------------------------------------------------*\
    | Probe GetInfo to build zone bitmask.                      |
    | Single query returns 14 bytes covering zone IDs 0-111.    |
    | Testing shows all tested devices return identical data     |
    | for typeOfInfo 0, 1, 2 — only query once.                 |
    \*---------------------------------------------------------*/
    uint8_t bitmap[14];
    memset(bitmap, 0, sizeof(bitmap));

    {
        uint8_t query[3] = { 0x00, 0x00, 0x00 };
        SendLong(perkey_idx, FN_8081_GET_INFO, query, 3);

        blankFAPmessage response;
        int result = ReadResponse(response, perkey_idx, FN_8081_GET_INFO);

        if(result > 0)
        {
            LOG_DEBUG("%s Per-key GetInfo response: "
                      "%02X %02X | %02X %02X %02X %02X %02X %02X %02X "
                      "%02X %02X %02X %02X %02X %02X %02X",
                      LOG_TAG,
                      response.data[0], response.data[1],
                      response.data[2], response.data[3], response.data[4],
                      response.data[5], response.data[6], response.data[7],
                      response.data[8], response.data[9], response.data[10],
                      response.data[11], response.data[12], response.data[13],
                      response.data[14], response.data[15]);

            memcpy(bitmap, response.data + 2, 14);
        }
    }

    /*---------------------------------------------------------*\
    | Parse bitmask to find valid zone IDs (0-111)              |
    \*---------------------------------------------------------*/
    caps.perkey_zone_ids.clear();

    for(int zone_id = 1; zone_id < 112; zone_id++)
    {
        int byte_idx = zone_id / 8;
        int bit_idx  = zone_id % 8;

        if(byte_idx < 14 && (bitmap[byte_idx] & (1 << bit_idx)))
        {
            caps.perkey_zone_ids.push_back((uint16_t)zone_id);
        }
    }

    caps.has_perkey = !caps.perkey_zone_ids.empty();

    /*---------------------------------------------------------*\
    | Detect numpad presence from zone bitmask.                  |
    | Numpad zones are 80-96 in Solaar's KEYCODES numbering.    |
    \*---------------------------------------------------------*/
    caps.has_numpad = false;

    for(uint16_t zid : caps.perkey_zone_ids)
    {
        if(zid >= 80 && zid <= 96)
        {
            caps.has_numpad = true;
            break;
        }
    }

    LOG_VERBOSE("%s Per-key zones discovered: %zu (numpad=%s)",
                LOG_TAG, caps.perkey_zone_ids.size(),
                caps.has_numpad ? "yes" : "no");
}

void LogitechHIDPPController::DiscoverKeyboardLayout()
{
    uint8_t idx = GetFeatureIndex(HIDPP_FEAT_KEYBOARD_LAYOUT);

    if(idx == 0)
    {
        caps.keyboard_layout_code = 0;
        return;
    }

    uint8_t recv_data[16] = {};
    int result = SendAndReceive(idx, 0x00, nullptr, 0, recv_data, sizeof(recv_data));

    if(result > 0)
    {
        caps.keyboard_layout_code = recv_data[0];
        LOG_DEBUG("%s Keyboard layout code: %d", LOG_TAG, caps.keyboard_layout_code);
    }
    else
    {
        caps.keyboard_layout_code = 0;
    }
}

/*---------------------------------------------------------*\
| Probe / Initialize / Shutdown                             |
\*---------------------------------------------------------*/

bool LogitechHIDPPController::Probe()
{
    LOG_DEBUG("%s Probing device at %s (index=0x%02X)",
              LOG_TAG, location.c_str(), device_index);

    /*---------------------------------------------------------*\
    | Flush any queued HID reports before probing.               |
    | The device may have unsolicited notifications (battery,    |
    | button events, etc.) sitting in the read buffer.           |
    \*---------------------------------------------------------*/
    {
        uint8_t flush_buf[64];
        int flushed = 0;

        while(flushed < 20)
        {
            int r = hid_read_timeout(dev, flush_buf, sizeof(flush_buf), 0);

            if(r <= 0)
            {
                break;
            }

            flushed++;
        }

        if(flushed > 0)
        {
            LOG_DEBUG("%s Flushed %d queued reports", LOG_TAG, flushed);
        }
    }

    /*---------------------------------------------------------*\
    | Test IRoot by looking up FeatureSet (0x0001).              |
    | Retry up to 3 times — wireless devices behind a shared     |
    | receiver can return stale responses if a report arrives    |
    | between our flush and query. FeatureSet MUST exist on      |
    | any HID++ 2.0 device, so index=0 means a bad read.        |
    \*---------------------------------------------------------*/
    uint8_t test_idx = 0;

    for(int attempt = 0; attempt < 3 && test_idx == 0; attempt++)
    {
        if(attempt > 0)
        {
            /*-------------------------------------------------*\
            | Flush again before retry — more reports may have   |
            | arrived since the last attempt.                    |
            \*-------------------------------------------------*/
            uint8_t retry_buf[64];

            while(hid_read_timeout(dev, retry_buf, sizeof(retry_buf), 10) > 0)
            {
            }

            LOG_DEBUG("%s IRoot retry %d at %s", LOG_TAG, attempt + 1, location.c_str());
        }

        test_idx = GetFeatureIndex(HIDPP_FEAT_FEATURE_SET);
    }

    if(test_idx == 0)
    {
        LOG_DEBUG("%s IRoot probe failed at %s — device does not support HID++ 2.0",
                  LOG_TAG, location.c_str());
        return false;
    }

    /*---------------------------------------------------------*\
    | If retries were needed, flush delayed responses from       |
    | failed attempts before continuing with discovery.          |
    \*---------------------------------------------------------*/
    {
        uint8_t post_buf[64];

        while(hid_read_timeout(dev, post_buf, sizeof(post_buf), 10) > 0)
        {
        }
    }

    /*---------------------------------------------------------*\
    | Discover device identity                                  |
    \*---------------------------------------------------------*/
    DiscoverDeviceName();
    DiscoverDeviceType();

    /*---------------------------------------------------------*\
    | Discover profile management features                      |
    \*---------------------------------------------------------*/
    caps.idx_profile_management = GetFeatureIndex(HIDPP_FEAT_PROFILE_MANAGEMENT);
    caps.idx_onboard_profiles   = GetFeatureIndex(HIDPP_FEAT_ONBOARD_PROFILES);

    /*---------------------------------------------------------*\
    | Discover RGB capabilities                                 |
    \*---------------------------------------------------------*/
    DiscoverRGBEffects();
    DiscoverPerKeyZones();
    DiscoverKeyboardLayout();

    if(!caps.has_zone_effects && !caps.has_perkey)
    {
        LOG_DEBUG("%s %s: no RGB features found", LOG_TAG, caps.device_name.c_str());
        return false;
    }

    LOG_VERBOSE("%s %s: zones=%zu perkey=%zu",
                LOG_TAG, caps.device_name.c_str(),
                caps.zone_clusters.size(), caps.perkey_zone_ids.size());

    return true;
}

void LogitechHIDPPController::Initialize()
{
    /*---------------------------------------------------------*\
    | Switch to host mode via ProfileManagement (0x8101) or      |
    | OnboardProfiles (0x8100).                                  |
    |                                                           |
    | ProfileManagement: fn6(0x05) = set_onboard=1, onboard=0   |
    |   (host), set_power=1, power=0 (disable FW power mgmt).   |
    |   Prevents firmware 10s idle timer from reasserting        |
    |   profile lighting.                                        |
    | OnboardProfiles: fn1(0x02) = host mode.                   |
    \*---------------------------------------------------------*/
    if(caps.idx_profile_management != 0)
    {
        uint8_t data[1] = { 0x05 };
        SendLong(caps.idx_profile_management, FN_8101_GET_SET_MODE, data, 1);

        blankFAPmessage response;
        ReadResponse(response, caps.idx_profile_management, FN_8101_GET_SET_MODE);

        LOG_VERBOSE("%s ProfileManagement set to host mode", LOG_TAG);
    }
    else if(caps.idx_onboard_profiles != 0)
    {
        uint8_t data[1] = { 0x02 };
        SendLong(caps.idx_onboard_profiles, FN_8100_SET_ONBOARD_MODE, data, 1);

        blankFAPmessage response;
        ReadResponse(response, caps.idx_onboard_profiles, FN_8100_SET_ONBOARD_MODE);

        LOG_VERBOSE("%s OnboardProfiles set to host mode", LOG_TAG);
    }

    /*---------------------------------------------------------*\
    | Claim SW control on RGB effects feature                    |
    | flags=5 (ZONE|EFFECT): firmware keeps POWER control,       |
    | monitors for idle, sends onUserActivity events.            |
    \*---------------------------------------------------------*/
    if(caps.idx_rgb_effects != 0)
    {
        SetSWControl(3, 5);
        SetRGBPowerMode(1);
    }

    initialized = true;
}

void LogitechHIDPPController::Shutdown()
{
    if(!initialized)
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Release SW control                                        |
    \*---------------------------------------------------------*/
    if(caps.idx_rgb_effects != 0)
    {
        SetSWControl(0, 0);
    }

    /*---------------------------------------------------------*\
    | Restore firmware mode                                     |
    \*---------------------------------------------------------*/
    if(caps.idx_profile_management != 0)
    {
        uint8_t data[1] = { 0x03 };
        SendLong(caps.idx_profile_management, FN_8101_GET_SET_MODE, data, 1);

        blankFAPmessage response;
        ReadResponse(response, caps.idx_profile_management, FN_8101_GET_SET_MODE);
    }
    else if(caps.idx_onboard_profiles != 0)
    {
        uint8_t data[1] = { 0x01 };
        SendLong(caps.idx_onboard_profiles, FN_8100_SET_ONBOARD_MODE, data, 1);

        blankFAPmessage response;
        ReadResponse(response, caps.idx_onboard_profiles, FN_8100_SET_ONBOARD_MODE);
    }

    initialized = false;
}

/*---------------------------------------------------------*\
| Accessors                                                 |
\*---------------------------------------------------------*/

const HIDPPDeviceCapabilities& LogitechHIDPPController::GetCapabilities() const
{
    return caps;
}

std::string LogitechHIDPPController::GetDeviceLocation()
{
    return "HID: " + location;
}

std::string LogitechHIDPPController::GetSerialString()
{
    return "";
}

/*---------------------------------------------------------*\
| SW Control and Power                                      |
\*---------------------------------------------------------*/

void LogitechHIDPPController::SetSWControl(uint8_t mode, uint8_t flags)
{
    if(caps.idx_rgb_effects == 0)
    {
        return;
    }

    uint8_t data[3] = { 0x01, mode, flags };
    SendLong(caps.idx_rgb_effects, FN_8071_SW_CONTROL, data, 3);

    blankFAPmessage response;
    ReadResponse(response, caps.idx_rgb_effects, FN_8071_SW_CONTROL);

    LOG_DEBUG("%s SetSWControl mode=%d flags=0x%02X", LOG_TAG, mode, flags);
}

void LogitechHIDPPController::SetRGBPowerMode(uint8_t mode)
{
    if(caps.idx_rgb_effects == 0)
    {
        return;
    }

    uint8_t data[2] = { 0x01, mode };
    SendLong(caps.idx_rgb_effects, FN_8071_PWR_MODE, data, 2);

    blankFAPmessage response;
    ReadResponse(response, caps.idx_rgb_effects, FN_8071_PWR_MODE);

    LOG_DEBUG("%s SetRGBPowerMode mode=%d", LOG_TAG, mode);
}

/*---------------------------------------------------------*\
| Per-key lighting (0x8081)                                 |
\*---------------------------------------------------------*/

void LogitechHIDPPController::SetPerKeyColors
    (
    const std::vector<std::pair<uint16_t, RGBColor>>& zone_colors
    )
{
    uint8_t perkey_idx = (caps.idx_perkey_v2 != 0) ? caps.idx_perkey_v2 : caps.idx_perkey_v1;

    if(perkey_idx == 0)
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Batch into SetIndividualRgbZones (fn1): 4 entries/packet  |
    | Each entry = [zone_id, R, G, B]                           |
    | Wait for ACK after each packet to prevent queue overrun.  |
    \*---------------------------------------------------------*/
    uint8_t data[16];
    uint8_t recv[16];
    int count = 0;

    for(size_t i = 0; i < zone_colors.size(); i++)
    {
        int offset = count * 4;
        data[offset + 0] = zone_colors[i].first;
        data[offset + 1] = RGBGetRValue(zone_colors[i].second);
        data[offset + 2] = RGBGetGValue(zone_colors[i].second);
        data[offset + 3] = RGBGetBValue(zone_colors[i].second);
        count++;

        if(count == 4 || i == zone_colors.size() - 1)
        {
            SendAndReceive(perkey_idx, FN_8081_SET_INDIVIDUAL, data, count * 4, recv, sizeof(recv));

            memset(data, 0, sizeof(data));
            count = 0;
        }
    }
}

void LogitechHIDPPController::SetAllPerKeyColor(RGBColor color)
{
    uint8_t perkey_idx = (caps.idx_perkey_v2 != 0) ? caps.idx_perkey_v2 : caps.idx_perkey_v1;

    if(perkey_idx == 0)
    {
        return;
    }

    LOG_DEBUG("%s SetAllPerKeyColor: R=%d G=%d B=%d zones=%zu",
              LOG_TAG, RGBGetRValue(color), RGBGetGValue(color), RGBGetBValue(color),
              caps.perkey_zone_ids.size());

    /*---------------------------------------------------------*\
    | Use SetRgbZonesSingleValue (fn6): R,G,B + up to 13 zones |
    \*---------------------------------------------------------*/
    uint8_t r = RGBGetRValue(color);
    uint8_t g = RGBGetGValue(color);
    uint8_t b = RGBGetBValue(color);

    for(size_t i = 0; i < caps.perkey_zone_ids.size(); i += 13)
    {
        uint8_t data[16];
        data[0] = r;
        data[1] = g;
        data[2] = b;

        size_t batch = caps.perkey_zone_ids.size() - i;
        if(batch > 13)
        {
            batch = 13;
        }

        for(size_t j = 0; j < batch; j++)
        {
            data[3 + j] = caps.perkey_zone_ids[i + j];
        }

        uint8_t recv[16];
        SendAndReceive(perkey_idx, FN_8081_SET_SINGLE_VALUE, data, 3 + batch, recv, sizeof(recv));
    }
}

void LogitechHIDPPController::SendPerKeyData
    (
    uint8_t         perkey_idx,
    uint8_t         function,
    const uint8_t*  data,
    size_t          len
    )
{
    uint8_t recv[16];
    SendAndReceive(perkey_idx, function, data, len, recv, sizeof(recv));
}

bool LogitechHIDPPController::PerKeyFrameEnd(bool persist)
{
    uint8_t perkey_idx = (caps.idx_perkey_v2 != 0) ? caps.idx_perkey_v2 : caps.idx_perkey_v1;

    if(perkey_idx == 0)
    {
        return false;
    }

    /*---------------------------------------------------------*\
    | Flush any unsolicited notifications (battery, button      |
    | events) that arrived during writes.                       |
    \*---------------------------------------------------------*/
    {
        uint8_t flush_buf[64];

        while(hid_read_timeout(dev, flush_buf, sizeof(flush_buf), 5) > 0)
        {
        }
    }

    frame_counter++;
    uint16_t fc = (uint16_t)(frame_counter & 0xFFFF);

    uint8_t data[3] = { (uint8_t)(persist ? 0x01 : 0x00),
                         (uint8_t)((fc >> 8) & 0xFF),
                         (uint8_t)(fc & 0xFF) };

    /*---------------------------------------------------------*\
    | Send FrameEnd and retry on BUSY without delay.            |
    | The G515 consistently returns BUSY once then succeeds     |
    | immediately — this appears to be normal device behavior,  |
    | not a timing issue.                                       |
    \*---------------------------------------------------------*/
    for(int attempt = 0; attempt < 5; attempt++)
    {
        SendMessage(perkey_idx, FN_8081_FRAME_END, data, sizeof(data));

        uint8_t resp_feat = 0, resp_func = 0;
        uint8_t resp_data[16] = {};

        int result = ReadMessage(&resp_feat, &resp_func, resp_data, sizeof(resp_data));

        if(result <= 0)
        {
            LOG_DEBUG("%s FrameEnd: no response (attempt %d)", LOG_TAG, attempt + 1);
            continue;
        }

        if(resp_feat == 0xFF && resp_data[1] == 0x08)
        {
            /*-------------------------------------------------*\
            | BUSY — device is processing previous frame commit. |
            | Wait briefly then retry.                           |
            \*-------------------------------------------------*/
            LOG_DEBUG("%s FrameEnd BUSY (attempt %d, fc=%u)", LOG_TAG, attempt + 1, frame_counter);
            std::this_thread::sleep_for(10ms);
            continue;
        }

        if(resp_feat == 0xFF)
        {
            LOG_DEBUG("%s FrameEnd error 0x%02X (attempt %d, fc=%u)",
                      LOG_TAG, resp_data[1], attempt + 1, frame_counter);
            continue;
        }

        /*-----------------------------------------------------*\
        | Success                                                |
        \*-----------------------------------------------------*/
        if(attempt > 0)
        {
            LOG_DEBUG("%s FrameEnd succeeded on attempt %d (fc=%u)",
                      LOG_TAG, attempt + 1, frame_counter);
        }
        return true;
    }

    LOG_DEBUG("%s FrameEnd failed after 5 attempts (fc=%u)", LOG_TAG, frame_counter);
    return false;
}

/*---------------------------------------------------------*\
| Zone effects (0x8071 / 0x8070)                            |
\*---------------------------------------------------------*/

void LogitechHIDPPController::SetZoneEffect
    (
    uint8_t         cluster_idx,
    uint8_t         effect_idx,
    uint16_t        effect_id,
    unsigned char   r,
    unsigned char   g,
    unsigned char   b,
    uint16_t        period,
    bool            /*persist*/
    )
{
    if(caps.idx_rgb_effects == 0)
    {
        return;
    }

    /*---------------------------------------------------------*\
    | SetEffectByIndex (fn1 on 0x8071, fn3 on 0x8070)           |
    | 0x8071: [cluster, effect_idx, 10-byte params, persist]    |
    | 0x8070: [zone, effect_idx, 11-byte params]                |
    \*---------------------------------------------------------*/
    uint8_t data[16];
    memset(data, 0, sizeof(data));

    data[0] = cluster_idx;
    data[1] = effect_idx;

    /*---------------------------------------------------------*\
    | Build 10-byte params (data[2..11]) per effect type         |
    | Layouts from protocol docs and LGHUB captures              |
    \*---------------------------------------------------------*/
    switch(effect_id)
    {
        case 0x0001:    // Static
            data[2] = r;
            data[3] = g;
            data[4] = b;
            break;

        case 0x000A:    // Breathing
            data[2] = r;
            data[3] = g;
            data[4] = b;
            data[5] = (period >> 8) & 0xFF;
            data[6] = period & 0xFF;
            data[8] = (period > 0) ? (uint8_t)(period / 100) : 50;
            break;

        case 0x0003:    // Color Cycle / Spectrum
            data[7] = (period >> 8) & 0xFF;
            data[8] = period & 0xFF;
            data[9] = (period > 0) ? (uint8_t)(period / 100) : 50;
            break;

        case 0x0004:    // Color Wave
            data[3] = (period > 0) ? (uint8_t)(period / 100) : 50;
            data[8] = 0x01;     // pattern
            data[9] = 0x00;     // waveform
            data[11] = 0x01;    // direction
            break;

        case 0x000B:    // Ripple
            data[2] = r;
            data[3] = g;
            data[4] = b;
            data[6] = (period >> 8) & 0xFF;
            data[7] = period & 0xFF;
            break;

        default:        // Unknown — best-effort
            data[2] = r;
            data[3] = g;
            data[4] = b;
            data[5] = (period >> 8) & 0xFF;
            data[6] = period & 0xFF;
            break;
    }

    if(caps.rgb_feature_page == HIDPP_FEAT_RGB_EFFECTS)
    {
        data[12] = 0x01;
        SendLong(caps.idx_rgb_effects, FN_8071_SET_EFFECT, data, 16);
    }
    else
    {
        SendLong(caps.idx_rgb_effects, LOGITECH_FP8070_SET_EFFECT, data, 13);
    }

    blankFAPmessage response;
    ReadResponse(response, caps.idx_rgb_effects,
                 (caps.rgb_feature_page == HIDPP_FEAT_RGB_EFFECTS) ? FN_8071_SET_EFFECT : LOGITECH_FP8070_SET_EFFECT);
}
