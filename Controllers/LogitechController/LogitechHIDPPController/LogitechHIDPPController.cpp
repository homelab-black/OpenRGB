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
#include <set>
#include <fstream>
#include <dirent.h>
#include "LogitechHIDPPController.h"
#include "RGBController_LogitechHIDPP.h"
#include "LogManager.h"

using namespace std::chrono_literals;

#define LOG_TAG log_tag.c_str()

/*-----------------------------------------------------------*\
| Hard cap on per-call non-HID++ drains in the read loop.    |
| A high-polling-rate mouse can put 50+ input reports in the |
| buffer between our reads; this cap prevents pathological   |
| input-flood scenarios from locking up a single read call.  |
| 64 is enough headroom for normal congestion at 1 kHz.     |
\*-----------------------------------------------------------*/
static const int HIDPP_READ_DRAIN_BUDGET = 64;

LogitechHIDPPController::LogitechHIDPPController
    (
    hid_device*                 dev,
    const char*                 path,
    uint8_t                     device_index,
    bool                        wireless,
    std::shared_ptr<std::mutex> mutex_ptr,
    uint16_t                    usage_page
    )
{
    this->dev           = dev;
    this->location      = path;
    this->device_index  = device_index;
    this->wireless      = wireless;
    this->mutex         = mutex_ptr;
    this->initialized          = false;
    this->sw_control_claimed   = false;
    this->frame_counter        = 0;
    this->init_generation      = 0;
    this->log_tag              = "[HID++ " + std::string(path) + "]";
    this->reader_thread        = nullptr;
    this->reader_running       = false;
    this->power_thread         = nullptr;
    this->power_thread_running = false;
    this->pending_activity     = -1;
    this->pending_connection   = 0;
    this->device_online        = true;
    this->consecutive_timeouts = 0;
    this->watcher_mode         = false;
    this->power_state          = HIDPP_POWER_ACTIVE;
    this->dim_brightness_pct   = 100;
    this->dim_step             = 0;
    this->idle_timeout_s       = 60;
    this->sleep_timeout_s      = 300;

    caps = {};

    /*---------------------------------------------------------*\
    | Default to standard HID++ transport; DiscoverTransport()   |
    | may change this during Probe() if Centurion is detected.   |
    \*---------------------------------------------------------*/
    transport.type              = HIDPP_TRANSPORT_STANDARD;
    transport.usage_page        = usage_page;
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
    /*---------------------------------------------------------*\
    | When the reader thread is running, it is the sole caller  |
    | of hid_read_timeout. All other reads come from the queue. |
    | Before the reader starts (during Probe/Initialize), read  |
    | directly from HID.                                        |
    \*---------------------------------------------------------*/
    if(reader_running.load())
    {
        return ReadFromQueue(feat_idx_out, function_out, data_out, data_max, timeout_ms);
    }

    return ReadHIDDirect(feat_idx_out, function_out, data_out, data_max, timeout_ms);
}

int LogitechHIDPPController::ReadHIDDirect
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
            return ReadCenturionDirect(feat_idx_out, function_out, data_out, data_max, timeout_ms);

        case HIDPP_TRANSPORT_STANDARD:
        default:
            return ReadStandardDirect(feat_idx_out, function_out, data_out, data_max, timeout_ms);
    }
}

int LogitechHIDPPController::ReadFromQueue
    (
    uint8_t*    feat_idx_out,
    uint8_t*    function_out,
    uint8_t*    data_out,
    size_t      data_max,
    int         timeout_ms
    )
{
    std::unique_lock<std::mutex> lock(response_mutex);

    auto deadline = std::chrono::steady_clock::now()
                  + std::chrono::milliseconds(timeout_ms);

    while(response_queue.empty())
    {
        if(response_cv.wait_until(lock, deadline) == std::cv_status::timeout)
        {
            int timeouts = ++consecutive_timeouts;

            /*-------------------------------------------------*\
            | 10 consecutive timeouts (~3s with 300ms each) before|
            | marking offline. Single delayed responses don't    |
            | trigger offline state.                             |
            \*-------------------------------------------------*/
            if(timeouts >= 10 && device_online.load())
            {
                LOG_DEBUG("%s Device appears offline (%d consecutive timeouts)",
                          LOG_TAG, timeouts);
                device_online.store(false);
            }

            return 0;
        }

        if(!reader_running.load())
        {
            return 0;
        }
    }

    consecutive_timeouts.store(0);

    HIDPPRawMessage msg = response_queue.front();
    response_queue.pop_front();

    if(feat_idx_out)
    {
        *feat_idx_out = msg.feat;
    }

    if(function_out)
    {
        *function_out = msg.func;
    }

    if(data_out && data_max > 0)
    {
        size_t copy_len = (data_max > sizeof(msg.data)) ? sizeof(msg.data) : data_max;
        memcpy(data_out, msg.data, copy_len);
    }

    return msg.result;
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
    /*---------------------------------------------------------*\
    | Thin wrapper around SendAcked with the reliable policy.    |
    | Preserved as a named entry point so existing call sites    |
    | don't need to be touched.                                  |
    \*---------------------------------------------------------*/
    return SendAcked(feat_idx, function,
                     send_data, send_len,
                     recv_data, recv_max,
                     HIDPP_POLICY_RELIABLE);
}

int LogitechHIDPPController::SendAcked
    (
    uint8_t                 feat_idx,
    uint8_t                 function,
    const uint8_t*          send_data,
    size_t                  send_len,
    uint8_t*                recv_data,
    size_t                  recv_max,
    const HIDPPRetryPolicy& policy,
    uint8_t*                hidpp_error_out
    )
{
    /*---------------------------------------------------------*\
    | Universal send-and-ack with policy-driven retry. Mirrors  |
    | the firmware's own event burst pattern: 7-attempt          |
    | exponential backoff for reliable one-shot commands, tight  |
    | 2-attempt for streaming animation frames.                  |
    |                                                           |
    | Loop semantics per attempt:                                |
    |   1. Sleep backoff_ms[i] (0 on first attempt)              |
    |   2. Bail if device went offline                          |
    |   3. SendMessage; on wire error, mark and retry            |
    |   4. Read loop bounded by read_window_ms:                  |
    |      - matching response  -> success                      |
    |      - HID++ error 0xFF for our request:                  |
    |          BUSY (0x08) + retry_on_busy -> retry the send    |
    |          other code                  -> hard fail (-1)    |
    |      - HID++ error for different request -> discard       |
    |      - non-matching, non-error frame    -> discard        |
    |      - read timeout (0)                  -> retry the send|
    \*---------------------------------------------------------*/
    if(hidpp_error_out)
    {
        *hidpp_error_out = 0;
    }

    if(policy.flush_before)
    {
        FlushResponseQueue();
    }

    int     last_result = 0;
    uint8_t last_error  = 0;

    for(uint8_t attempt = 0; attempt < policy.attempts; attempt++)
    {
        /*-----------------------------------------------------*\
        | Backoff before each attempt (0 on first)               |
        \*-----------------------------------------------------*/
        uint16_t delay_ms = policy.backoff_ms[attempt];

        if(delay_ms > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        }

        /*-----------------------------------------------------*\
        | Bail early if device went offline mid-retry            |
        \*-----------------------------------------------------*/
        if(!device_online.load())
        {
            return 0;
        }

        int send_result = SendMessage(feat_idx, function, send_data, send_len);

        if(send_result < 0)
        {
            LOG_DEBUG("%s SendAcked[%s] wire send failed (attempt %d, result=%d) "
                      "feat=0x%02X func=0x%02X",
                      LOG_TAG, policy.name, attempt, send_result, feat_idx, function);
            last_result = -2;
            continue;
        }

        /*-----------------------------------------------------*\
        | Read loop bounded by per-attempt window. Drain        |
        | non-matching HID++ frames within this window — they   |
        | are stale responses or unrelated events from prior    |
        | commands. Only retry the send if the window expires   |
        | with no match (lost on wire) or we got BUSY.           |
        \*-----------------------------------------------------*/
        auto window_deadline = std::chrono::steady_clock::now()
                             + std::chrono::milliseconds(policy.read_window_ms);
        bool need_resend = false;

        while(!need_resend)
        {
            auto now = std::chrono::steady_clock::now();

            if(now >= window_deadline)
            {
                LOG_TRACE("%s SendAcked[%s] window expired (attempt %d)",
                          LOG_TAG, policy.name, attempt);
                last_result = 0;
                break;
            }

            int remaining = (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                                  window_deadline - now).count();

            if(remaining <= 0)
            {
                last_result = 0;
                break;
            }

            uint8_t resp_feat = 0;
            uint8_t resp_func = 0;
            uint8_t resp_data[60] = {};

            int rd = ReadMessage(&resp_feat, &resp_func,
                                 resp_data, sizeof(resp_data),
                                 remaining);

            if(rd < 0)
            {
                /* Wire error — propagate, don't retry */
                return -2;
            }

            if(rd == 0)
            {
                /* Window drained with nothing matching — retry the send */
                last_result = 0;
                break;
            }

            /*-------------------------------------------------*\
            | HID++ error frame                                 |
            | feat=0xFF, func=err_feat, data[0]=err_func,       |
            | data[1]=err_code                                  |
            \*-------------------------------------------------*/
            if(resp_feat == 0xFF)
            {
                uint8_t err_feat = resp_func;
                uint8_t err_func = resp_data[0];
                uint8_t err_code = resp_data[1];

                if(err_feat == feat_idx &&
                   (err_func & 0xF0) == (function & 0xF0))
                {
                    if(err_code == 0x08 && policy.retry_on_busy)
                    {
                        /* BUSY: retry the send after backoff */
                        LOG_TRACE("%s SendAcked[%s] BUSY (attempt %d) feat=0x%02X func=0x%02X",
                                  LOG_TAG, policy.name, attempt, feat_idx, function);
                        last_error  = 0x08;
                        last_result = 0;
                        need_resend = true;
                        continue;
                    }

                    /* Non-BUSY HID++ error: hard fail */
                    LOG_DEBUG("%s SendAcked[%s] HID++ error 0x%02X "
                              "feat=0x%02X func=0x%02X",
                              LOG_TAG, policy.name, err_code, feat_idx, function);

                    if(hidpp_error_out)
                    {
                        *hidpp_error_out = err_code;
                    }

                    return -1;
                }

                /* Error for a different request — stale, discard and keep reading */
                continue;
            }

            /*-------------------------------------------------*\
            | Match our expected response                       |
            \*-------------------------------------------------*/
            if(resp_feat == feat_idx &&
               (resp_func & 0xF0) == (function & 0xF0))
            {
                if(recv_data && recv_max > 0)
                {
                    size_t copy = (recv_max > sizeof(resp_data))
                                ? sizeof(resp_data) : recv_max;
                    memcpy(recv_data, resp_data, copy);
                }

                if(attempt > 0)
                {
                    LOG_DEBUG("%s SendAcked[%s] succeeded on attempt %d "
                              "feat=0x%02X func=0x%02X",
                              LOG_TAG, policy.name, attempt, feat_idx, function);
                }

                return rd;
            }

            /* Non-matching, non-error: stale unrelated frame, keep reading */
        }
    }

    LOG_DEBUG("%s SendAcked[%s] exhausted %d attempts feat=0x%02X func=0x%02X "
              "(last_error=0x%02X)",
              LOG_TAG, policy.name, (int)policy.attempts,
              feat_idx, function, last_error);

    if(hidpp_error_out)
    {
        *hidpp_error_out = last_error;
    }

    return last_result;
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

    /* Discard non-matching message and retry */
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
    /*---------------------------------------------------------*\
    | Auto-select short (0x10, 7 bytes) vs long (0x11, 20 bytes) |
    | based on data length. Upper layers just provide data;       |
    | transport picks the smallest frame that fits.               |
    \*---------------------------------------------------------*/
    uint8_t buf[LOGITECH_LONG_MESSAGE_LEN];
    size_t  msg_len;

    if(len <= 3)
    {
        memset(buf, 0, LOGITECH_SHORT_MESSAGE_LEN);
        buf[0] = LOGITECH_SHORT_MESSAGE;
        buf[1] = device_index;
        buf[2] = feat_idx;
        buf[3] = function | HIDPP_SW_ID;

        if(data && len > 0)
        {
            memcpy(buf + 4, data, len);
        }

        msg_len = LOGITECH_SHORT_MESSAGE_LEN;
    }
    else
    {
        memset(buf, 0, LOGITECH_LONG_MESSAGE_LEN);
        buf[0] = LOGITECH_LONG_MESSAGE;
        buf[1] = device_index;
        buf[2] = feat_idx;
        buf[3] = function | HIDPP_SW_ID;

        if(data && len > 0)
        {
            size_t copy_len = (len > 16) ? 16 : len;
            memcpy(buf + 4, data, copy_len);
        }

        msg_len = LOGITECH_LONG_MESSAGE_LEN;
    }

    {
        static auto t0 = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();

        if(msg_len == LOGITECH_SHORT_MESSAGE_LEN)
        {
            LOG_TRACE("%s [%lld.%03lld] <= w[%02X %02X %02X%02X %02X%02X%02X]",
                      LOG_TAG, (long long)(ms / 1000), (long long)(ms % 1000),
                      buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6]);
        }
        else
        {
            LOG_TRACE("%s [%lld.%03lld] <= w[%02X %02X %02X%02X %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X]",
                      LOG_TAG, (long long)(ms / 1000), (long long)(ms % 1000),
                      buf[0], buf[1], buf[2], buf[3],
                      buf[4], buf[5], buf[6], buf[7],
                      buf[8], buf[9], buf[10], buf[11],
                      buf[12], buf[13], buf[14], buf[15],
                      buf[16], buf[17], buf[18], buf[19]);
        }
    }

    int result;

    if(mutex)
    {
        std::lock_guard<std::mutex> lock(*mutex);
        result = hid_write(dev, buf, msg_len);
    }
    else
    {
        result = hid_write(dev, buf, msg_len);
    }

    return result;
}

int LogitechHIDPPController::ReadStandardDirect
    (
    uint8_t*    feat_idx_out,
    uint8_t*    function_out,
    uint8_t*    data_out,
    size_t      data_max,
    int         timeout_ms
    )
{
    /*---------------------------------------------------------*\
    | No mutex needed for reads — when the reader thread is      |
    | running, it is the sole caller. Before the reader starts,  |
    | all access is single-threaded.                             |
    |                                                           |
    | Loop within the timeout window draining non-HID++ reports |
    | (mouse motion, keystrokes, media keys, DJ events) until    |
    | we either find a HID++ short/long frame or actually time   |
    | out. A high-polling-rate device can put 50+ input reports  |
    | in the hidraw buffer between our calls; without the drain  |
    | loop the synchronous probe path can never get past them    |
    | to find its response.                                      |
    \*---------------------------------------------------------*/
    auto deadline = std::chrono::steady_clock::now()
                  + std::chrono::milliseconds(timeout_ms);
    int  drained  = 0;

    while(true)
    {
        auto now = std::chrono::steady_clock::now();

        if(now >= deadline)
        {
            return 0;
        }

        int remaining_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                                deadline - now).count();

        if(remaining_ms <= 0)
        {
            return 0;
        }

        blankFAPmessage response;
        response.init();

        int result = hid_read_timeout(dev, response.buffer, response.size(), remaining_ms);

        if(result < 0)
        {
            /* Real wire error (e.g. device removed). */
            return result;
        }

        if(result == 0)
        {
            /* hidapi timeout — window expired with nothing pending. */
            return 0;
        }

        /*-----------------------------------------------------*\
        | Validate report ID. The hidraw can carry HID input    |
        | reports (keyboard, mouse, media keys) in addition to  |
        | HID++. Drop anything that isn't a HID++ short (0x10)  |
        | or long (0x11) message and keep draining within the   |
        | remaining window — otherwise the parser would read    |
        | buf[2]/buf[3] as feat/func and misinterpret           |
        | keystrokes/motion as HID++ events.                    |
        \*-----------------------------------------------------*/
        if(response.buffer[0] != LOGITECH_SHORT_MESSAGE &&
           response.buffer[0] != LOGITECH_LONG_MESSAGE)
        {
            if(++drained > HIDPP_READ_DRAIN_BUDGET)
            {
                LOG_DEBUG("%s ReadStandardDirect: drain budget (%d) exceeded",
                          LOG_TAG, HIDPP_READ_DRAIN_BUDGET);
                return 0;
            }
            continue;
        }

        {
            static auto t0 = std::chrono::steady_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
            LOG_TRACE("%s [%lld.%03lld] => r[%02X %02X %02X%02X %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X]",
                      LOG_TAG, (long long)(ms / 1000), (long long)(ms % 1000),
                      response.buffer[0], response.buffer[1], response.buffer[2], response.buffer[3],
                      response.buffer[4], response.buffer[5], response.buffer[6], response.buffer[7],
                      response.buffer[8], response.buffer[9], response.buffer[10], response.buffer[11],
                      response.buffer[12], response.buffer[13], response.buffer[14], response.buffer[15],
                      response.buffer[16], response.buffer[17], response.buffer[18], response.buffer[19]);
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
            buf[1] = 5 + sub_msg_len;              // cplLen: flags(1) + feat(1) + func(1) + hdr(2) + sub
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
            buf[1] = 3 + (uint8_t)len;             // cplLen: flags(1) + feat(1) + func(1) + data
            buf[2] = 0x00;                          // flags
            buf[3] = feat_idx;
            buf[4] = function | HIDPP_SW_ID;

            if(data && len > 0)
            {
                memcpy(buf + 5, data, len);
            }
        }
    }

    {
        static auto t0 = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
        LOG_TRACE("%s [%lld.%03lld] <= w[%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ...]",
                  LOG_TAG, (long long)(ms / 1000), (long long)(ms % 1000),
                  buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
                  buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
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

int LogitechHIDPPController::ReadCenturionDirect
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

    /*---------------------------------------------------------*\
    | Track an overall deadline so the bridge ACK + MessageEvent |
    | two-read sequence stays within timeout_ms total — without  |
    | this each read could eat the full budget independently.    |
    | Drain non-Centurion report IDs within the remaining window |
    | rather than bailing on the first non-matching frame.       |
    \*---------------------------------------------------------*/
    auto deadline = std::chrono::steady_clock::now()
                  + std::chrono::milliseconds(timeout_ms);
    int  drained  = 0;
    int  result   = 0;

    while(true)
    {
        auto now = std::chrono::steady_clock::now();

        if(now >= deadline)
        {
            return 0;
        }

        int remaining_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                                deadline - now).count();

        if(remaining_ms <= 0)
        {
            return 0;
        }

        result = hid_read_timeout(dev, buf, sizeof(buf), remaining_ms);

        if(result < 0)
        {
            return result;
        }

        if(result == 0)
        {
            return 0;
        }

        if(buf[0] == transport.report_id)
        {
            break;
        }

        if(++drained > HIDPP_READ_DRAIN_BUDGET)
        {
            LOG_DEBUG("%s ReadCenturionDirect: drain budget (%d) exceeded",
                      LOG_TAG, HIDPP_READ_DRAIN_BUDGET);
            return 0;
        }
    }

    {
        static auto t0 = std::chrono::steady_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
        LOG_TRACE("%s [%lld.%03lld] => r[%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ...]",
                  LOG_TAG, (long long)(ms / 1000), (long long)(ms % 1000),
                  buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
                  buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
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
        | CentPPBridge — distinguish events from command responses|
        |                                                        |
        | Bridge events (e.g. ConnectionStateChangedEvent) have  |
        | func high nibble = 0x00 (event index 0) and swid = 0. |
        | These are NOT wrapped sub-device responses — they are  |
        | bridge-level notifications. Return as-is so the reader |
        | thread can detect them.                                |
        |                                                        |
        | Command responses follow a two-response pattern:       |
        | 1. ACK: bridge echoes feat+func with our swid          |
        | 2. MessageEvent: func=1x, swid=0, wrapped sub-device   |
        \*-----------------------------------------------------*/
        if((resp_func & 0xF0) == 0x00 && (resp_func & 0x0F) != HIDPP_SW_ID)
        {
            /*-------------------------------------------------*\
            | Bridge event — return feat/func/data as-is         |
            \*-------------------------------------------------*/
            if(feat_idx_out)   *feat_idx_out = resp_feat;
            if(function_out)   *function_out = resp_func;

            if(data_out && data_max > 0)
            {
                size_t avail = (size_t)(cpl_len > 2 ? cpl_len - 2 : 0);
                size_t copy = (avail < data_max) ? avail : data_max;
                memcpy(data_out, buf + 5 + hdr_offset, copy);
            }

            return result;
        }

        if((resp_func & 0x0F) == HIDPP_SW_ID)
        {
            /*-------------------------------------------------*\
            | This is the ACK — discard and read the MessageEvent|
            | Use the *remaining* window from the overall        |
            | deadline so the two-read sequence stays bounded,   |
            | and drain non-Centurion frames within that window. |
            \*-------------------------------------------------*/
            while(true)
            {
                auto now = std::chrono::steady_clock::now();

                if(now >= deadline)
                {
                    return 0;
                }

                int remaining_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(
                                        deadline - now).count();

                if(remaining_ms <= 0)
                {
                    return 0;
                }

                memset(buf, 0, sizeof(buf));
                result = hid_read_timeout(dev, buf, sizeof(buf), remaining_ms);

                if(result < 0)
                {
                    return result;
                }

                if(result == 0)
                {
                    return 0;
                }

                if(buf[0] == transport.report_id)
                {
                    break;
                }

                if(++drained > HIDPP_READ_DRAIN_BUDGET)
                {
                    LOG_DEBUG("%s ReadCenturionDirect: drain budget (%d) exceeded on bridge MessageEvent",
                              LOG_TAG, HIDPP_READ_DRAIN_BUDGET);
                    return 0;
                }
            }

            {
                static auto t0 = std::chrono::steady_clock::now();
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - t0).count();
                LOG_TRACE("%s [%lld.%03lld] => r[%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X ...]",
                          LOG_TAG, (long long)(ms / 1000), (long long)(ms % 1000),
                          buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
                          buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);
            }

            resp_feat = buf[3 + hdr_offset];
            resp_func = buf[4 + hdr_offset];

            if(resp_feat != transport.bridge_feat_idx)
            {
                /*---------------------------------------------*\
                | Not a bridge response — return as-is           |
                \*---------------------------------------------*/
                if(feat_idx_out)   *feat_idx_out = resp_feat;
                if(function_out)   *function_out = resp_func;

                if(data_out && data_max > 0)
                {
                    size_t avail = (size_t)(buf[1 + hdr_offset] > 2 ? buf[1 + hdr_offset] - 2 : 0);
                    size_t copy = (avail < data_max) ? avail : data_max;
                    memcpy(data_out, buf + 5 + hdr_offset, copy);
                }

                return result;
            }
        }

        /*-----------------------------------------------------*\
        | MessageEvent — unwrap sub-device response.             |
        | Bridge params: [devId<<4|lenHi, lenLo, subCPL,         |
        |                 subFeatIdx, subFunc|swid, subData...]   |
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
    /*---------------------------------------------------------*\
    | Check cache first — both Centurion bulk and HID++ on-demand|
    | lookups store results here.                                |
    \*---------------------------------------------------------*/
    auto it = caps.feature_map.find(feature_page);

    if(it != caps.feature_map.end())
    {
        return it->second;
    }

    /*---------------------------------------------------------*\
    | Centurion bulk enumeration is complete — if a feature      |
    | isn't in the map, it doesn't exist. No wire query needed.  |
    \*---------------------------------------------------------*/
    if(caps.feature_map_complete)
    {
        return 0;
    }

    /*---------------------------------------------------------*\
    | Standard HID++: on-demand IRoot query, cache the result.   |
    \*---------------------------------------------------------*/
    uint8_t send_data[2];
    send_data[0] = (feature_page >> 8) & 0xFF;
    send_data[1] = feature_page & 0xFF;

    uint8_t recv_data[16] = {};
    int result = SendAndReceive(LOGITECH_HIDPP_PAGE_ROOT_IDX, FN_8071_GET_INFO,
                                send_data, 2, recv_data, sizeof(recv_data));

    if(result > 0)
    {
        uint8_t index = recv_data[0];

        if(index != 0)
        {
            caps.feature_map[feature_page] = index;
        }

        LOG_DEBUG("%s Feature 0x%04X -> index 0x%02X", LOG_TAG, feature_page, index);
        return index;
    }

    /*---------------------------------------------------------*\
    | Cache the miss too so we don't re-query failed lookups     |
    \*---------------------------------------------------------*/
    caps.feature_map[feature_page] = 0;

    LOG_DEBUG("%s Feature 0x%04X not found", LOG_TAG, feature_page);
    return 0;
}

void LogitechHIDPPController::DiscoverDeviceName()
{
    /*---------------------------------------------------------*\
    | Centurion sub-devices use 0x0101 (DeviceName).             |
    | Standard HID++ uses 0x0005 (DeviceNameType).               |
    \*---------------------------------------------------------*/
    if(transport.type == HIDPP_TRANSPORT_CENTURION)
    {
        /*-----------------------------------------------------*\
        | Centurion sub-device: 0x0101 getName returns firmware   |
        | data, not a readable name. Read from sysfs HID_NAME    |
        | instead (same approach as Solaar).                      |
        \*-----------------------------------------------------*/
        std::string sysfs_name;
        std::string hidraw = location;
        size_t pos = hidraw.rfind("hidraw");

        if(pos != std::string::npos)
        {
            std::string uevent_path = "/sys/class/hidraw/" + hidraw.substr(pos) + "/device/uevent";
            FILE* f = fopen(uevent_path.c_str(), "r");

            if(f)
            {
                char line[256];

                while(fgets(line, sizeof(line), f))
                {
                    if(strncmp(line, "HID_NAME=", 9) == 0)
                    {
                        sysfs_name = line + 9;

                        while(!sysfs_name.empty() && (sysfs_name.back() == '\n' || sysfs_name.back() == '\r'))
                        {
                            sysfs_name.pop_back();
                        }

                        break;
                    }
                }

                fclose(f);
            }
        }

        if(!sysfs_name.empty())
        {
            caps.device_name = sysfs_name;
        }
        else
        {
            caps.device_name = "Logitech Centurion Device";
        }

        LOG_VERBOSE("%s Device name (Centurion): %s", LOG_TAG, caps.device_name.c_str());
        return;
    }

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
    /*---------------------------------------------------------*\
    | Centurion sub-devices don't have 0x0005 (DeviceNameType).  |
    | Default to unknown — don't assume device type from         |
    | transport, as Centurion may be used for future devices.    |
    \*---------------------------------------------------------*/
    if(transport.type == HIDPP_TRANSPORT_CENTURION)
    {
        caps.device_type = 0;
        return;
    }

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
    | Detect transport type from usage page.                     |
    | 0xFF00/0xFF43: Standard HID++ (0x10/0x11 reports)          |
    | 0xFFA0+:       Centurion (64-byte CPL framing)             |
    \*---------------------------------------------------------*/
    if(transport.usage_page == 0xFF00 || transport.usage_page == 0xFF43)
    {
        transport.type = HIDPP_TRANSPORT_STANDARD;
        return;
    }

    /*---------------------------------------------------------*\
    | Centurion transport — determine variant from the report    |
    | descriptor. 0x51 = direct (PRO X 2), 0x50 = addressed     |
    | (G522). Parse the descriptor to find which report IDs      |
    | are available with 63-byte payload.                        |
    \*---------------------------------------------------------*/
    transport.type = HIDPP_TRANSPORT_CENTURION;

    /*---------------------------------------------------------*\
    | Parse report descriptor to find Centurion report IDs       |
    \*---------------------------------------------------------*/
    uint8_t desc[4096];
    int desc_len = hid_get_report_descriptor(dev, desc, sizeof(desc));

    bool has_0x51 = false;
    bool has_0x50 = false;

    if(desc_len > 0)
    {
        for(int i = 0; i < desc_len; i++)
        {
            if(desc[i] == 0x85 && i + 1 < desc_len)
            {
                if(desc[i + 1] == 0x51) has_0x51 = true;
                if(desc[i + 1] == 0x50) has_0x50 = true;
            }
        }
    }

    if(has_0x51)
    {
        transport.report_id = 0x51;
        transport.addressed = false;
        LOG_DEBUG("%s Centurion 0x51 (direct) from descriptor", LOG_TAG);
    }
    else if(has_0x50)
    {
        transport.report_id = 0x50;
        transport.addressed = true;
        transport.device_address = 0x00;

        /*-----------------------------------------------------*\
        | Read the device address from the first response.       |
        | Send a CenturionRoot probe and read back.              |
        \*-----------------------------------------------------*/
        uint8_t probe_buf[64] = {};
        probe_buf[0] = 0x50;
        probe_buf[1] = 0x00;
        probe_buf[2] = 3;
        probe_buf[3] = 0x00;
        probe_buf[4] = 0x00;
        probe_buf[5] = 0x00 | HIDPP_SW_ID;

        if(hid_write(dev, probe_buf, 64) > 0)
        {
            uint8_t resp_buf[64] = {};

            if(hid_read_timeout(dev, resp_buf, sizeof(resp_buf), 500) > 0)
            {
                transport.device_address = resp_buf[1];
            }
        }

        LOG_DEBUG("%s Centurion 0x50 (addressed) from descriptor, device_addr=0x%02X",
                  LOG_TAG, transport.device_address);
    }
    else
    {
        /*-----------------------------------------------------*\
        | Descriptor didn't reveal report IDs — fall back to     |
        | probing 0x51 then 0x50 by trial.                       |
        \*-----------------------------------------------------*/
        transport.report_id = 0x51;
        transport.addressed = false;

        uint8_t probe_buf[64] = {};
        probe_buf[0] = 0x51;
        probe_buf[1] = 3;
        probe_buf[2] = 0x00;
        probe_buf[3] = 0x00;
        probe_buf[4] = 0x00 | HIDPP_SW_ID;

        int wr = hid_write(dev, probe_buf, 64);

        if(wr > 0)
        {
            uint8_t resp_buf[64] = {};
            int rd = hid_read_timeout(dev, resp_buf, sizeof(resp_buf), 500);

            if(rd > 0 && resp_buf[0] == 0x51)
            {
                LOG_DEBUG("%s Centurion 0x51 (direct) from probe", LOG_TAG);
                return;
            }
        }

        transport.report_id = 0x50;
        transport.addressed = true;
        transport.device_address = 0x00;

        memset(probe_buf, 0, sizeof(probe_buf));
        probe_buf[0] = 0x50;
        probe_buf[1] = 0x00;
        probe_buf[2] = 3;
        probe_buf[3] = 0x00;
        probe_buf[4] = 0x00;
        probe_buf[5] = 0x00 | HIDPP_SW_ID;

        wr = hid_write(dev, probe_buf, 64);

        if(wr > 0)
        {
            uint8_t resp_buf[64] = {};
            int rd = hid_read_timeout(dev, resp_buf, sizeof(resp_buf), 500);

            if(rd > 0 && resp_buf[0] == 0x50)
            {
                transport.device_address = resp_buf[1];
                LOG_DEBUG("%s Centurion 0x50 (addressed) from probe, device_addr=0x%02X",
                          LOG_TAG, transport.device_address);
                return;
            }
        }

        LOG_DEBUG("%s Centurion transport: no response (usage_page=0x%04X)",
                  LOG_TAG, transport.usage_page);
    }
}

void LogitechHIDPPController::EnumerateFeatures(uint8_t feature_set_idx)
{
    caps.feature_map.clear();
    caps.feature_map_complete = false;

    /*---------------------------------------------------------*\
    | Root (0x0000) is always at index 0                         |
    \*---------------------------------------------------------*/
    caps.feature_map[0x0000] = 0;

    if(transport.type == HIDPP_TRANSPORT_CENTURION)
    {
        /*-----------------------------------------------------*\
        | Centurion sub-device: CenturionFeatureSet fn1 returns  |
        | ALL features in a single bulk response.                |
        | [count, (feat_hi, feat_lo, type, flags) × count]       |
        \*-----------------------------------------------------*/
        uint8_t send_data[1] = { 0x00 };
        uint8_t recv_data[60] = {};

        int result = SendAndReceive(feature_set_idx, 0x10,
                                    send_data, 1, recv_data, sizeof(recv_data));

        if(result > 0)
        {
            uint8_t count = recv_data[0];

            LOG_DEBUG("%s CenturionFeatureSet: %d features", LOG_TAG, count);

            for(uint8_t i = 0; i < count && (1 + i * 4 + 1) < (int)sizeof(recv_data); i++)
            {
                int offset = 1 + i * 4;
                uint16_t feat_id = ((uint16_t)recv_data[offset] << 8) | recv_data[offset + 1];
                uint8_t  feat_type = recv_data[offset + 2];
                uint8_t  feat_idx  = i;  // 0-based: bulk includes root at 0

                caps.feature_map[feat_id] = feat_idx;

                LOG_DEBUG("%s   [%2d] Feature 0x%04X type=0x%02X",
                          LOG_TAG, feat_idx, feat_id, feat_type);
            }

            caps.feature_map_complete = true;
        }
    }
    else
    {
        /*-----------------------------------------------------*\
        | Standard HID++: no bulk query available. Features are  |
        | looked up on-demand via GetFeatureIndex (IRoot) and    |
        | cached in the feature map. Nothing to do here.         |
        \*-----------------------------------------------------*/
        return;
    }
}

void LogitechHIDPPController::DiscoverFirmwareInfo()
{
    /*---------------------------------------------------------*\
    | Centurion sub-devices use 0x0100 (DeviceInfo) for firmware  |
    | version and serial. Standard HID++ uses 0x0003 (FirmwareInfo).|
    \*---------------------------------------------------------*/
    if(transport.type == HIDPP_TRANSPORT_CENTURION)
    {
        uint8_t dev_info_idx = GetFeatureIndex(HIDPP_FEAT_CENTURION_DEVICE_INFO);

        if(dev_info_idx == 0)
        {
            return;
        }

        /*-----------------------------------------------------*\
        | fn1 getFirmwareVersion(entityIndex=0) — main firmware   |
        | Response: [fwType, additional, version_hi, version_lo]  |
        \*-----------------------------------------------------*/
        {
            uint8_t send_data[1] = { 0x00 };
            uint8_t recv_data[16] = {};

            int result = SendAndReceive(dev_info_idx, 0x10,
                                        send_data, 1, recv_data, sizeof(recv_data));

            if(result > 0)
            {
                uint16_t version = ((uint16_t)recv_data[2] << 8) | recv_data[3];

                char ver_str[32];
                snprintf(ver_str, sizeof(ver_str), "%d.%d",
                         (version >> 8) & 0xFF, version & 0xFF);

                caps.firmware_version = ver_str;

                LOG_DEBUG("%s Firmware (Centurion): %s", LOG_TAG, caps.firmware_version.c_str());
            }
        }

        /*-----------------------------------------------------*\
        | fn2 getSerialNumber on 0x0100 (DeviceInfo)              |
        | Response: [stringLen, serial...]                        |
        \*-----------------------------------------------------*/
        {
            uint8_t recv_data[16] = {};

            int result = SendAndReceive(dev_info_idx, 0x20,
                                        nullptr, 0, recv_data, sizeof(recv_data));

            if(result > 0)
            {
                uint8_t slen = recv_data[0];
                if(slen > 15) slen = 15;

                caps.serial_number = std::string((char*)&recv_data[1], slen);

                LOG_DEBUG("%s Serial (Centurion): %s", LOG_TAG, caps.serial_number.c_str());
            }
        }

        return;
    }

    uint8_t fw_idx = GetFeatureIndex(HIDPP_FEAT_FIRMWARE_INFO);

    if(fw_idx == 0)
    {
        return;
    }

    /*---------------------------------------------------------*\
    | fn0 GetEntityCount — entity count, unitId, transport PIDs  |
    | Response: [count, unitId(4), transport(2), PID1(2), PID2(2)]|
    \*---------------------------------------------------------*/
    uint8_t entity_count = 1;

    {
        uint8_t recv_data[16] = {};
        int result = SendAndReceive(fw_idx, 0x00,
                                    nullptr, 0, recv_data, sizeof(recv_data));

        if(result > 0)
        {
            entity_count = recv_data[0];

            /*-------------------------------------------------*\
            | Extract unitId — stable hardware identity across   |
            | all connection paths (USB, wireless, dongle).      |
            \*-------------------------------------------------*/
            char uid[16];
            snprintf(uid, sizeof(uid), "%02X%02X%02X%02X",
                     recv_data[1], recv_data[2], recv_data[3], recv_data[4]);
            caps.unit_id = uid;

            caps.pid_wireless = ((uint16_t)recv_data[7] << 8) | recv_data[8];
            caps.pid_wired    = ((uint16_t)recv_data[9] << 8) | recv_data[10];

            /*-------------------------------------------------*\
            | Use unitId as serial if device doesn't report one  |
            \*-------------------------------------------------*/
            if(caps.serial_number.empty() && caps.unit_id != "00000000")
            {
                caps.serial_number = caps.unit_id;
            }

            LOG_DEBUG("%s unitId=%s PID1=0x%04X PID2=0x%04X",
                      LOG_TAG, caps.unit_id.c_str(), caps.pid_wireless, caps.pid_wired);
        }
    }

    /*---------------------------------------------------------*\
    | fn1 GetFwInfo — iterate entities to find main FW (type 0)  |
    | fwType lower nibble: 0=main, 1=bootloader, 2=HW rev       |
    | Response: fwType(1), prefix(3), bcdVersion(2), bcdBuild(2) |
    \*---------------------------------------------------------*/
    for(uint8_t entity = 0; entity < entity_count && entity < 8; entity++)
    {
        uint8_t send_data[1] = { entity };
        uint8_t recv_data[16] = {};

        int result = SendAndReceive(fw_idx, 0x10,
                                    send_data, 1, recv_data, sizeof(recv_data));

        if(result <= 0)
        {
            continue;
        }

        uint8_t fw_type   = recv_data[0] & 0x0F;
        char prefix[4]    = { (char)recv_data[1], (char)recv_data[2], (char)recv_data[3], '\0' };
        uint8_t ver_major = recv_data[4];
        uint8_t ver_minor = recv_data[5];
        uint16_t build    = ((uint16_t)recv_data[6] << 8) | recv_data[7];

        char ver_str[64];
        snprintf(ver_str, sizeof(ver_str), "%s %d.%d.%05u",
                 prefix, ver_major, ver_minor, build);

        LOG_DEBUG("%s Firmware entity %d: type=%d %s", LOG_TAG, entity, fw_type, ver_str);

        if(fw_type == 0)
        {
            caps.firmware_version = ver_str;
        }
    }

    if(caps.firmware_version.empty())
    {
        LOG_DEBUG("%s No main firmware entity found", LOG_TAG);
    }

    /*---------------------------------------------------------*\
    | fn2 GetDeviceSerialNumber — ASCII serial up to 16 bytes    |
    \*---------------------------------------------------------*/
    {
        uint8_t recv_data[16] = {};

        int result = SendAndReceive(fw_idx, 0x20,
                                    nullptr, 0, recv_data, sizeof(recv_data));

        if(result > 0)
        {
            char serial[17] = {};
            memcpy(serial, recv_data, 16);
            serial[16] = '\0';

            /*-------------------------------------------------*\
            | Trim trailing nulls/spaces                        |
            \*-------------------------------------------------*/
            for(int i = 15; i >= 0; i--)
            {
                if(serial[i] == '\0' || serial[i] == ' ')
                {
                    serial[i] = '\0';
                }
                else
                {
                    break;
                }
            }

            if(serial[0] != '\0')
            {
                caps.serial_number = serial;
            }

            LOG_DEBUG("%s Serial: %s", LOG_TAG, caps.serial_number.c_str());
        }
    }
}

void LogitechHIDPPController::DiscoverRGBEffects()
{
    /*---------------------------------------------------------*\
    | Try 0x8071 first, then 0x0600 (Centurion), then 0x8070    |
    \*---------------------------------------------------------*/
    caps.idx_rgb_effects = GetFeatureIndex(HIDPP_FEAT_RGB_EFFECTS);
    caps.rgb_feature_page = HIDPP_FEAT_RGB_EFFECTS;

    if(caps.idx_rgb_effects == 0)
    {
        caps.idx_rgb_effects = GetFeatureIndex(HIDPP_FEAT_CENTURION_RGB);
        caps.rgb_feature_page = HIDPP_FEAT_CENTURION_RGB;
    }

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
    | Resolve function IDs based on which feature was found.     |
    | 0x8071 and 0x0600 share the same function layout.          |
    | 0x8070 has different function numbers and SW control format.|
    \*---------------------------------------------------------*/
    if(caps.rgb_feature_page == HIDPP_FEAT_COLOR_LED_EFFECTS)
    {
        caps.fn_set_effect      = 0x30;
        caps.fn_sw_control      = 0x80;
        caps.fn_pwr_config      = 0;
        caps.fn_pwr_mode        = 0;
        caps.has_power_mgmt     = false;
        caps.sw_control_simple  = true;
    }
    else
    {
        caps.fn_set_effect      = 0x10;
        caps.fn_sw_control      = 0x50;
        caps.fn_pwr_config      = 0x70;
        caps.fn_pwr_mode        = 0x80;
        caps.has_power_mgmt     = true;
        caps.sw_control_simple  = false;
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
    | Detect transport type from usage page before anything else.|
    | Centurion devices need different framing for all commands.  |
    \*---------------------------------------------------------*/
    DiscoverTransport();

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
    | Test IRoot by looking up a known feature.                  |
    |                                                           |
    | Standard HID++: look up FeatureSet (0x0001) — must exist. |
    | Centurion dongle: look up CentPPBridge (0x0003) — the     |
    |   dongle doesn't have FeatureSet, but must have the        |
    |   bridge to reach the sub-device.                          |
    |                                                           |
    | Retry up to 3 times — wireless devices behind a shared     |
    | receiver can return stale responses.                       |
    \*---------------------------------------------------------*/
    uint8_t test_idx = 0;

    if(transport.type == HIDPP_TRANSPORT_CENTURION)
    {
        /*-----------------------------------------------------*\
        | Centurion: try CentPPBridge (0x0003) first for dongle. |
        | If not found, try FeatureSet (0x0001) for wired/direct |
        | connection where the device IS the endpoint.           |
        \*-----------------------------------------------------*/
        test_idx = GetFeatureIndex(HIDPP_FEAT_CENTPPBRIDGE);

        if(test_idx != 0)
        {
            transport.bridge_feat_idx = test_idx;
            transport.sub_device_id   = 0;

            LOG_DEBUG("%s CentPPBridge at index %d — routing to sub-device",
                      LOG_TAG, test_idx);
        }
        else
        {
            LOG_DEBUG("%s No CentPPBridge — Centurion direct connection", LOG_TAG);
            test_idx = GetFeatureIndex(HIDPP_FEAT_FEATURE_SET);
        }
    }
    else
    {
        /*-----------------------------------------------------*\
        | Standard HID++: probe FeatureSet (0x0001).             |
        | Retry up to 3 times for wireless devices.              |
        \*-----------------------------------------------------*/
        for(int attempt = 0; attempt < 3 && test_idx == 0; attempt++)
        {
            if(attempt > 0)
            {
                uint8_t retry_buf[64];

                while(hid_read_timeout(dev, retry_buf, sizeof(retry_buf), 10) > 0)
                {
                }

                LOG_DEBUG("%s IRoot retry %d at %s", LOG_TAG, attempt + 1, location.c_str());
            }

            test_idx = GetFeatureIndex(HIDPP_FEAT_FEATURE_SET);
        }
    }

    if(test_idx == 0)
    {
        LOG_DEBUG("%s IRoot probe failed at %s — device does not respond",
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
    | Enumerate all features in bulk. For standard HID++, uses   |
    | FeatureSet GetCount + GetFeatureId loop. For Centurion     |
    | sub-devices, uses bulk GetFeatureId (single response).     |
    | After this, GetFeatureIndex uses the map — no wire traffic.|
    \*---------------------------------------------------------*/
    {
        /*-----------------------------------------------------*\
        | For Centurion bridged, FeatureSet is at index 1 on the |
        | sub-device. For standard HID++, test_idx is the        |
        | FeatureSet index from the IRoot probe.                 |
        \*-----------------------------------------------------*/
        uint8_t fs_idx = (transport.type == HIDPP_TRANSPORT_CENTURION &&
                          transport.bridge_feat_idx != 0)
                       ? 1    // CenturionFeatureSet always at index 1 on sub-device
                       : test_idx;

        EnumerateFeatures(fs_idx);

        /*-----------------------------------------------------*\
        | If bridged and bulk enumeration failed, the sub-device |
        | isn't reachable (e.g., headset off or on USB cable).   |
        | Mark map as complete so lookups don't hit the wire.    |
        | The device will be discovered with no features — it    |
        | can be re-probed when the sub-device comes online.     |
        \*-----------------------------------------------------*/
        if(transport.bridge_feat_idx != 0 && !caps.feature_map_complete)
        {
            LOG_DEBUG("%s Sub-device not reachable through bridge — dongle only",
                      LOG_TAG);
            caps.feature_map_complete = true;
        }
    }

    /*---------------------------------------------------------*\
    | Discover device identity.                                  |
    | On Centurion with bridge, this now queries the sub-device  |
    | (headset) through the bridge, not the dongle.              |
    \*---------------------------------------------------------*/
    DiscoverDeviceName();
    log_tag = "[HID++ " + caps.device_name + "]";
    DiscoverDeviceType();
    DiscoverFirmwareInfo();

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

    /*---------------------------------------------------------*\
    | Probe WirelessStatus (0x1D4B) for reconnect detection.    |
    | Lightspeed devices behind kernel-managed receivers send   |
    | WirelessStatus events when they reconnect after power     |
    | cycle. Cache the feature index so the reader thread can   |
    | detect these events without sending commands.             |
    \*---------------------------------------------------------*/
    caps.idx_wireless_status = GetFeatureIndex(HIDPP_FEAT_WIRELESS_STATUS);

    if(!caps.has_zone_effects && !caps.has_perkey)
    {
        LOG_DEBUG("%s %s: no RGB features found", LOG_TAG, caps.device_name.c_str());

        /*-----------------------------------------------------*\
        | Centurion dongles with bridge stay alive to watch for  |
        | sub-device connection events, even without RGB.        |
        \*-----------------------------------------------------*/
        if(transport.bridge_feat_idx != 0)
        {
            return true;
        }

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
    | No device state changes here — let firmware effects keep   |
    | running until DeviceUpdateMode claims control with real    |
    | colors ready via ClaimSWControlIfNeeded().                  |
    \*---------------------------------------------------------*/
    init_generation++;
    initialized = true;
}

void LogitechHIDPPController::Shutdown()
{
    if(!initialized)
    {
        return;
    }

    StopPowerManager();

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
    return caps.serial_number;
}

uint32_t LogitechHIDPPController::GetInitGeneration() const
{
    return init_generation;
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

    if(caps.sw_control_simple)
    {
        /*-----------------------------------------------------*\
        | 0x8070: simple [enabled(bool), persist(bool)]          |
        \*-----------------------------------------------------*/
        uint8_t data[2] = { (uint8_t)(mode > 0 ? 0x01 : 0x00), 0x00 };
        SendLong(caps.idx_rgb_effects, caps.fn_sw_control, data, 2);
    }
    else
    {
        /*-----------------------------------------------------*\
        | 0x8071/0x0600: [0x01(set), mode, flags]                |
        \*-----------------------------------------------------*/
        uint8_t data[3] = { 0x01, mode, flags };
        SendLong(caps.idx_rgb_effects, caps.fn_sw_control, data, 3);
    }

    blankFAPmessage response;
    ReadResponse(response, caps.idx_rgb_effects, caps.fn_sw_control);

    LOG_DEBUG("%s SetSWControl mode=%d flags=0x%02X", LOG_TAG, mode, flags);
}

void LogitechHIDPPController::SetRGBPowerMode(uint8_t mode)
{
    if(caps.idx_rgb_effects == 0 || !caps.has_power_mgmt)
    {
        return;
    }

    uint8_t data[2] = { 0x01, mode };
    SendLong(caps.idx_rgb_effects, caps.fn_pwr_mode, data, 2);

    blankFAPmessage response;
    ReadResponse(response, caps.idx_rgb_effects, caps.fn_pwr_mode);

    LOG_DEBUG("%s SetRGBPowerMode mode=%d", LOG_TAG, mode);
}

void LogitechHIDPPController::SetHostMode()
{
    if(caps.idx_profile_management != 0)
    {
        uint8_t data[1] = { 0x05 };
        SendLong(caps.idx_profile_management, FN_8101_GET_SET_MODE, data, 1);

        blankFAPmessage response;
        ReadResponse(response, caps.idx_profile_management, FN_8101_GET_SET_MODE);

        LOG_DEBUG("%s ProfileManagement set to host mode", LOG_TAG);
    }
    else if(caps.idx_onboard_profiles != 0)
    {
        uint8_t data[1] = { 0x02 };
        SendLong(caps.idx_onboard_profiles, FN_8100_SET_ONBOARD_MODE, data, 1);

        blankFAPmessage response;
        ReadResponse(response, caps.idx_onboard_profiles, FN_8100_SET_ONBOARD_MODE);

        LOG_DEBUG("%s OnboardProfiles set to host mode", LOG_TAG);
    }
}

void LogitechHIDPPController::ClaimSWControlIfNeeded()
{
    if(sw_control_claimed || caps.idx_rgb_effects == 0 || !device_online.load())
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Full takeover in documented order (matches Solaar):         |
    | 1. Host mode                                               |
    | 2. SW control — flags=5 directly for idle monitoring       |
    | 3. Power mode active                                       |
    \*---------------------------------------------------------*/
    /*---------------------------------------------------------*\
    | GHUB's exact claim sequence (capture lines 905-918):      |
    |   1. SetSWControl(3, 7)                                    |
    |   2. WritePowerConfig — uncompensated sleep               |
    |   3. SetRGBPowerMode(1)                                    |
    |   4. SetHostMode(5)                                        |
    |   5. WritePowerConfig — compensated sleep                 |
    |   6. SetSWControl(3, 5)                                    |
    |                                                           |
    | The two WritePowerConfig calls bracket SetHostMode: the    |
    | first sets the user-facing sleep value while still in      |
    | firmware-mode interpretation, the second pulls it forward  |
    | by the off-ramp duration (so the firmware's fade ends at  |
    | the user-expected sleep time) once we're in host mode.    |
    \*---------------------------------------------------------*/
    uint16_t effective_sleep = sleep_timeout_s;

    if(caps.nv_sleep_ramp_known && caps.nv_sleep_ramp_enabled
       && caps.nv_sleep_ramp_seconds < sleep_timeout_s)
    {
        effective_sleep -= caps.nv_sleep_ramp_seconds;
    }

    SetSWControl(3, 7);
    WritePowerConfig(idle_timeout_s, sleep_timeout_s);
    SetRGBPowerMode(1);
    SetHostMode();
    WritePowerConfig(idle_timeout_s, effective_sleep);
    SetSWControl(3, 5);

    sw_control_claimed = true;

    LOG_DEBUG("%s Claimed SW control (deferred from init)", LOG_TAG);
}

/*---------------------------------------------------------*\
| Per-key lighting (0x8081)                                 |
\*---------------------------------------------------------*/

void LogitechHIDPPController::SetPerKeyColors
    (
    const std::vector<std::pair<uint16_t, RGBColor>>& zone_colors
    )
{
    if(!device_online.load()) return;

    uint8_t perkey_idx = (caps.idx_perkey_v2 != 0) ? caps.idx_perkey_v2 : caps.idx_perkey_v1;

    if(perkey_idx == 0)
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Batch into SetIndividualRgbZones (fn1): 4 entries/packet  |
    | Each entry = [zone_id, R, G, B]                           |
    | Fire-and-forget: FrameEnd handles draining stale ACKs.    |
    \*---------------------------------------------------------*/
    uint8_t data[16];
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
            SendPerKeyData(perkey_idx, FN_8081_SET_INDIVIDUAL, data, count * 4);
            memset(data, 0, sizeof(data));
            count = 0;
        }
    }
}

void LogitechHIDPPController::SetAllPerKeyColor(RGBColor color)
{
    if(!device_online.load()) return;

    uint8_t perkey_idx = (caps.idx_perkey_v2 != 0) ? caps.idx_perkey_v2 : caps.idx_perkey_v1;

    if(perkey_idx == 0)
    {
        return;
    }

    LOG_DEBUG("%s SetAllPerKeyColor: R=%d G=%d B=%d zones=%zu",
              LOG_TAG, RGBGetRValue(color), RGBGetGValue(color), RGBGetBValue(color),
              caps.perkey_zone_ids.size());

    uint8_t r = RGBGetRValue(color);
    uint8_t g = RGBGetGValue(color);
    uint8_t b = RGBGetBValue(color);

    /*---------------------------------------------------------*\
    | Use SetRangeRgbZones (fn5): [start, end, R, G, B] × 3     |
    | per packet. Sets all zones in a contiguous range to one   |
    | color. Gaps in zone IDs are silently ignored by firmware.  |
    | For uniform color this is far more efficient than fn6:     |
    | 1-2 packets vs 8 packets for 94 zones.                    |
    \*---------------------------------------------------------*/
    uint8_t min_zone = 255, max_zone = 0;

    for(uint16_t zid : caps.perkey_zone_ids)
    {
        if(zid > 0 && zid <= 255)
        {
            if((uint8_t)zid < min_zone) min_zone = (uint8_t)zid;
            if((uint8_t)zid > max_zone) max_zone = (uint8_t)zid;
        }
    }

    if(min_zone <= max_zone)
    {
        uint8_t data[5] = { min_zone, max_zone, r, g, b };
        SendPerKeyData(perkey_idx, FN_8081_SET_RANGE, data, 5);
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
    SendLong(perkey_idx, function, data, len);

    /*---------------------------------------------------------*\
    | Wait for the ACK before sending the next write. This      |
    | matches GHUB's behavior — without it, the device gets     |
    | overrun and FrameEnd returns BUSY. Discard any            |
    | non-matching messages (events, stale acks) until we see   |
    | the matching ACK or time out.                             |
    \*---------------------------------------------------------*/
    for(int i = 0; i < 10; i++)
    {
        uint8_t resp_feat = 0;
        uint8_t resp_func = 0;
        uint8_t resp_data[16] = {};
        int result = ReadMessage(&resp_feat, &resp_func, resp_data, sizeof(resp_data), 50);

        if(result <= 0)
        {
            return;
        }

        if(resp_feat == perkey_idx && (resp_func & 0xF0) == (function & 0xF0))
        {
            return;
        }

        if(resp_feat == 0xFF)
        {
            /* HID++ error response — give up */
            return;
        }
    }
}

bool LogitechHIDPPController::PerKeyFrameEnd(bool persist)
{
    if(!device_online.load()) return false;

    uint8_t perkey_idx = (caps.idx_perkey_v2 != 0) ? caps.idx_perkey_v2 : caps.idx_perkey_v1;

    if(perkey_idx == 0)
    {
        return false;
    }

    /*---------------------------------------------------------*\
    | FrameEnd format (matches GHUB capture):                   |
    |   - LONG message (0x11), 16 bytes of zeros                |
    |   - No frame counter, no persist flag                     |
    | We were sending a SHORT message with [persist, fc_hi, fc_lo] |
    | which caused intermittent BUSY errors. GHUB never sees    |
    | BUSY because firmware expects long-format FrameEnd.       |
    \*---------------------------------------------------------*/
    uint8_t data[16] = {};
    (void)persist;  // ignored — GHUB doesn't use this

    /*---------------------------------------------------------*\
    | Send FrameEnd, then read responses in a loop.             |
    | Per-key write ACKs may be queued — PerKeyFrameEnd reads   |
    | and discards them while looking for the FrameEnd response.|
    | Retry on BUSY (device still processing previous commit).  |
    \*---------------------------------------------------------*/
    uint8_t frame_end_func = FN_8081_FRAME_END | HIDPP_SW_ID;

    for(int attempt = 0; attempt < 10; attempt++)
    {
        if(!device_online.load()) return false;

        SendMessage(perkey_idx, FN_8081_FRAME_END, data, sizeof(data));

        for(int reads = 0; reads < 20; reads++)
        {
            uint8_t resp_feat = 0, resp_func = 0;
            uint8_t resp_data[16] = {};

            int result = ReadMessage(&resp_feat, &resp_func, resp_data, sizeof(resp_data));

            if(result <= 0)
            {
                LOG_DEBUG("%s FrameEnd: read timeout (attempt %d, read %d)",
                          LOG_TAG, attempt + 1, reads + 1);
                break;
            }

            /*-------------------------------------------------*\
            | HID++ error response                               |
            \*-------------------------------------------------*/
            if(resp_feat == 0xFF)
            {
                /*---------------------------------------------*\
                | Check if this error is for OUR FrameEnd        |
                | resp_func = feat_idx, resp_data[0] = func|sw  |
                \*---------------------------------------------*/
                if(resp_func == perkey_idx && resp_data[0] == frame_end_func)
                {
                    if(resp_data[1] == 0x08)
                    {
                        LOG_DEBUG("%s FrameEnd BUSY (attempt %d, fc=%u)",
                                  LOG_TAG, attempt + 1, frame_counter);
                        std::this_thread::sleep_for(10ms);
                        break;
                    }

                    LOG_DEBUG("%s FrameEnd error 0x%02X (attempt %d, fc=%u)",
                              LOG_TAG, resp_data[1], attempt + 1, frame_counter);
                    break;
                }

                /*---------------------------------------------*\
                | Error for a different function (stale per-key  |
                | write error) — discard and keep reading.       |
                \*---------------------------------------------*/
                continue;
            }

            /*-------------------------------------------------*\
            | Normal response — check if it matches FrameEnd    |
            \*-------------------------------------------------*/
            if(resp_feat == perkey_idx &&
               (resp_func & 0xF0) == (FN_8081_FRAME_END & 0xF0))
            {
                if(attempt > 0)
                {
                    LOG_DEBUG("%s FrameEnd succeeded on attempt %d (fc=%u)",
                              LOG_TAG, attempt + 1, frame_counter);
                }
                return true;
            }

            /* Discard non-matching response (stale ACK etc.) */
        }
    }

    LOG_DEBUG("%s FrameEnd failed after 10 attempts (fc=%u)", LOG_TAG, frame_counter);
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
    bool            persist
    )
{
    if(caps.idx_rgb_effects == 0 || !device_online.load())
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
            /*-----------------------------------------------------*\
            | "Fixed color" marker — only set when there's an       |
            | actual color. All-black means "Off / pass-through to  |
            | per-key buffer", which uses byte 5 = 0x00 instead.    |
            \*-----------------------------------------------------*/
            if(r != 0 || g != 0 || b != 0)
            {
                data[5] = 0x02;
            }
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

    if(caps.rgb_feature_page == HIDPP_FEAT_COLOR_LED_EFFECTS)
    {
        /*-----------------------------------------------------*\
        | 0x8070: 13-byte payload, no persist byte               |
        \*-----------------------------------------------------*/
        SendLong(caps.idx_rgb_effects, caps.fn_set_effect, data, 13);
    }
    else
    {
        /*-----------------------------------------------------*\
        | 0x8071/0x0600: 16-byte payload with persist at [12]    |
        \*-----------------------------------------------------*/
        data[12] = persist ? 0x01 : 0x00;
        SendLong(caps.idx_rgb_effects, caps.fn_set_effect, data, 16);
    }

    blankFAPmessage response;
    ReadResponse(response, caps.idx_rgb_effects, caps.fn_set_effect);
}

/*---------------------------------------------------------*\
| Power management (idle/dim/sleep/wake)                    |
|                                                           |
| Matches Solaar's RGBPowerManager state machine:           |
|   ACTIVE → DIMMING → IDLE → SLEEPING                     |
|                                                           |
| Uses firmware onUserActivity events from 0x8071 for       |
| idle/active detection. SW control flags cycle:            |
|   7 (init) → 5 (active, monitor idle) →                  |
|   3 (idle, monitor active) → 5 (wake)                    |
\*---------------------------------------------------------*/

void LogitechHIDPPController::SetRepaintCallback(std::function<void()> repaint)
{
    request_repaint_fn = repaint;
}

void LogitechHIDPPController::SetReinitCallback(std::function<void()> cb)
{
    reinit_direct_fn = cb;
}

void LogitechHIDPPController::SetRegisterCallback(std::function<void(RGBController*)> cb)
{
    register_controller_fn = cb;
}

HIDPPPowerState LogitechHIDPPController::GetPowerState() const
{
    return power_state;
}

int LogitechHIDPPController::GetDimBrightness() const
{
    return dim_brightness_pct.load();
}

bool LogitechHIDPPController::HasBridge() const
{
    return transport.bridge_feat_idx != 0;
}

bool LogitechHIDPPController::IsOnline() const
{
    return device_online.load();
}

void LogitechHIDPPController::ReprobeSubDevice()
{
    /*---------------------------------------------------------*\
    | Called by power thread when a sub-device connects through  |
    | the Centurion bridge. The reader thread is running, so all |
    | commands go through SendAndRead → ReadFromQueue.           |
    |                                                           |
    | We clear the sub-device feature cache and re-discover      |
    | everything. The bridge_feat_idx and dongle name are kept.  |
    \*---------------------------------------------------------*/
    LOG_DEBUG("%s Re-probing sub-device through bridge", LOG_TAG);

    /*---------------------------------------------------------*\
    | Give the sub-device a moment to settle after connection    |
    | before sending commands through the bridge.               |
    \*---------------------------------------------------------*/
    std::this_thread::sleep_for(500ms);

    FlushResponseQueue();

    /*---------------------------------------------------------*\
    | Clear sub-device feature map but keep bridge index.        |
    | This forces fresh lookups through the bridge.             |
    \*---------------------------------------------------------*/
    caps.feature_map.clear();
    caps.feature_map_complete = false;
    caps.has_zone_effects     = false;
    caps.has_perkey            = false;
    caps.zone_clusters.clear();
    caps.perkey_zone_ids.clear();
    caps.idx_rgb_effects       = 0;
    caps.idx_perkey_v2          = 0;
    caps.idx_perkey_v1          = 0;
    caps.idx_profile_management = 0;
    caps.idx_onboard_profiles   = 0;
    caps.fn_set_effect          = 0;
    caps.fn_sw_control          = 0;
    caps.fn_pwr_config          = 0;
    caps.fn_pwr_mode            = 0;
    caps.has_power_mgmt         = false;
    caps.sw_control_simple      = false;
    caps.nv_sleep_ramp_known    = false;
    caps.nv_sleep_ramp_enabled  = false;
    caps.nv_sleep_ramp_seconds  = 0;

    /*---------------------------------------------------------*\
    | Re-populate feature map. CenturionFeatureSet is always at |
    | index 1 on the sub-device.                                |
    \*---------------------------------------------------------*/
    EnumerateFeatures(1);

    if(!caps.feature_map_complete)
    {
        LOG_DEBUG("%s Sub-device not reachable after connect event", LOG_TAG);
        caps.feature_map_complete = true;
        return;
    }

    /*---------------------------------------------------------*\
    | Discover sub-device identity if not already known.        |
    \*---------------------------------------------------------*/
    if(caps.device_name.empty() || caps.device_name.find("PRO X 2") == std::string::npos)
    {
        /*-----------------------------------------------------*\
        | Dongle may have a sysfs-derived name; get real name    |
        | from the sub-device now that it's reachable.           |
        \*-----------------------------------------------------*/
        std::string old_name = caps.device_name;
        DiscoverDeviceName();

        if(caps.device_name != old_name)
        {
            log_tag = "[HID++ " + caps.device_name + "]";
        }
    }

    DiscoverDeviceType();
    DiscoverFirmwareInfo();

    /*---------------------------------------------------------*\
    | Discover RGB features                                     |
    \*---------------------------------------------------------*/
    caps.idx_profile_management = GetFeatureIndex(HIDPP_FEAT_PROFILE_MANAGEMENT);
    caps.idx_onboard_profiles   = GetFeatureIndex(HIDPP_FEAT_ONBOARD_PROFILES);

    DiscoverRGBEffects();
    DiscoverPerKeyZones();
    DiscoverKeyboardLayout();

    if(!caps.has_zone_effects && !caps.has_perkey)
    {
        LOG_DEBUG("%s Sub-device has no RGB features", LOG_TAG);
        return;
    }

    LOG_INFO("%s Sub-device probed: zones=%zu perkey=%zu",
             LOG_TAG, caps.zone_clusters.size(), caps.perkey_zone_ids.size());

    /*---------------------------------------------------------*\
    | Create and register RGBController for the sub-device.     |
    \*---------------------------------------------------------*/
    Initialize();

    if(register_controller_fn)
    {
        RGBController_LogitechHIDPP* rgb = new RGBController_LogitechHIDPP(this);
        register_controller_fn(rgb);

        LOG_INFO("%s Registered RGB controller for sub-device", LOG_TAG);
    }
}

void LogitechHIDPPController::ReconnectDevice()
{
    /*---------------------------------------------------------*\
    | Called by power thread when WirelessStatus reconnect event |
    | arrives. Push colors ASAP to override the firmware boot    |
    | animation — same behavior as GHUB.                        |
    \*---------------------------------------------------------*/
    LOG_DEBUG("%s Reconnecting device", LOG_TAG);

    FlushResponseQueue();

    if(!device_online.load())
    {
        /*-----------------------------------------------------*\
        | First reconnect event (reconnect=1, config_needed=1).  |
        | Full init: reset state, claim SW control, push colors. |
        \*-----------------------------------------------------*/
        device_online.store(true);
        consecutive_timeouts.store(0);
        sw_control_claimed = false;
        frame_counter = 0;

        {
            std::lock_guard<std::mutex> lock(power_mutex);
            dim_brightness_pct.store(100);
            power_state = HIDPP_POWER_ACTIVE;
        }

        if(reinit_direct_fn)
        {
            reinit_direct_fn();
        }

        if(caps.has_power_mgmt)
        {
            ReadFirmwareTimers();
            ReadNvSleepRampConfig();
        }

        LOG_INFO("%s Device reconnected — colors pushed", LOG_TAG);
    }
    else
    {
        /*-----------------------------------------------------*\
        | Second event (config_needed=0) — firmware boot done.   |
        | Re-push colors in case firmware overwrote per-key      |
        | buffer during its initialization.                      |
        \*-----------------------------------------------------*/
        if(reinit_direct_fn)
        {
            reinit_direct_fn();
        }

        LOG_INFO("%s Device reconnected — colors re-pushed after firmware boot", LOG_TAG);
    }
}

void LogitechHIDPPController::FullReprobe()
{
    /*---------------------------------------------------------*\
    | Called by power thread when a failed-probe device becomes  |
    | reachable. Like ReprobeSubDevice but for non-bridge        |
    | standard HID++ devices. Reader thread is running.         |
    \*---------------------------------------------------------*/
    LOG_DEBUG("%s Full re-probe of previously unreachable device", LOG_TAG);

    FlushResponseQueue();

    caps.feature_map.clear();
    caps.feature_map_complete = false;
    caps.has_zone_effects      = false;
    caps.has_perkey             = false;
    caps.zone_clusters.clear();
    caps.perkey_zone_ids.clear();
    caps.idx_rgb_effects        = 0;
    caps.idx_perkey_v2           = 0;
    caps.idx_perkey_v1           = 0;
    caps.idx_wireless_status     = 0;
    caps.idx_profile_management  = 0;
    caps.idx_onboard_profiles    = 0;
    caps.fn_set_effect           = 0;
    caps.fn_sw_control           = 0;
    caps.fn_pwr_config           = 0;
    caps.fn_pwr_mode             = 0;
    caps.has_power_mgmt          = false;
    caps.sw_control_simple       = false;
    caps.nv_sleep_ramp_known     = false;
    caps.nv_sleep_ramp_enabled   = false;
    caps.nv_sleep_ramp_seconds   = 0;

    /*---------------------------------------------------------*\
    | Standard HID++ features are looked up on-demand — no bulk |
    | enumeration needed. Just re-discover everything.          |
    \*---------------------------------------------------------*/
    DiscoverDeviceName();
    log_tag = "[HID++ " + caps.device_name + "]";
    DiscoverDeviceType();
    DiscoverFirmwareInfo();

    caps.idx_profile_management = GetFeatureIndex(HIDPP_FEAT_PROFILE_MANAGEMENT);
    caps.idx_onboard_profiles   = GetFeatureIndex(HIDPP_FEAT_ONBOARD_PROFILES);
    caps.idx_wireless_status    = GetFeatureIndex(HIDPP_FEAT_WIRELESS_STATUS);

    DiscoverRGBEffects();
    DiscoverPerKeyZones();
    DiscoverKeyboardLayout();

    if(!caps.has_zone_effects && !caps.has_perkey)
    {
        LOG_DEBUG("%s Device has no RGB features after re-probe", LOG_TAG);
        return;
    }

    LOG_INFO("%s Re-probe complete: zones=%zu perkey=%zu",
             LOG_TAG, caps.zone_clusters.size(), caps.perkey_zone_ids.size());

    device_online.store(true);
    consecutive_timeouts.store(0);
    watcher_mode.store(false);
    Initialize();

    if(register_controller_fn)
    {
        RGBController_LogitechHIDPP* rgb = new RGBController_LogitechHIDPP(this);
        register_controller_fn(rgb);
        LOG_INFO("%s Registered RGB controller after re-probe", LOG_TAG);
    }
}

void LogitechHIDPPController::StartProbeWatcher()
{
    /*---------------------------------------------------------*\
    | Start reader + power threads in watcher mode for a device |
    | that failed initial probe. The power thread periodically  |
    | retries IRoot until the device becomes reachable.         |
    \*---------------------------------------------------------*/
    if(reader_running)
    {
        return;
    }

    watcher_mode.store(true);
    device_online.store(false);

    pending_connection = 0;
    reader_running = true;
    reader_thread = new std::thread(&LogitechHIDPPController::ReaderThreadFunc, this);

    power_thread_running = true;
    power_thread = new std::thread(&LogitechHIDPPController::PowerThreadFunc, this);

    LOG_DEBUG("%s Probe watcher started (retrying every 5s)", LOG_TAG);
}

bool LogitechHIDPPController::ScanForDevice()
{
    /*---------------------------------------------------------*\
    | Scan sysfs for a hidraw with matching unitId. Called by    |
    | the power thread when the device goes offline (USB/wireless|
    | switch). Finds the device on its new connection path.      |
    |                                                           |
    | Match criteria:                                           |
    |   - HID_UNIQ matches our unitId                           |
    |   - Logitech VID (046D)                                   |
    |   - HID++ interface (usage_page 0xFF00, usage 2)          |
    |   - Different from our current path (device moved)        |
    \*---------------------------------------------------------*/
    if(caps.unit_id.empty() || caps.unit_id == "00000000")
    {
        return false;
    }

    LOG_DEBUG("%s Scanning for device (unitId=%s, current=%s)",
              LOG_TAG, caps.unit_id.c_str(), location.c_str());

    /*---------------------------------------------------------*\
    | Normalize our unitId to lowercase hex without dashes for   |
    | comparison. Sysfs HID_UNIQ varies between drivers:         |
    |   - Lightspeed virtual: "0d-12-5d-47" (dashes)             |
    |   - USB direct: "0D125D47" (no dashes)                     |
    | We strip dashes and lowercase both sides for matching.     |
    \*---------------------------------------------------------*/
    std::string target_norm;

    for(char c : caps.unit_id)
    {
        if(c != '-')
        {
            target_norm += (char)tolower(c);
        }
    }

    /*---------------------------------------------------------*\
    | Read our current PID from sysfs. Only migrate to paths    |
    | with a DIFFERENT PID — same PID means same receiver,      |
    | just a different pairing slot (e.g., stale pairing).      |
    \*---------------------------------------------------------*/
    unsigned int current_pid = 0;
    {
        std::string cur_hidraw = location.substr(location.rfind('/') + 1);
        std::string cur_uevent = "/sys/class/hidraw/" + cur_hidraw + "/device/uevent";
        std::ifstream cur_file(cur_uevent);
        std::string line;

        while(std::getline(cur_file, line))
        {
            if(line.compare(0, 7, "HID_ID=") == 0)
            {
                size_t lc = line.rfind(':');

                if(lc != std::string::npos)
                {
                    sscanf(line.c_str() + lc + 1, "%x", &current_pid);
                }

                break;
            }
        }
    }

    DIR* dir = opendir("/sys/class/hidraw");

    if(!dir)
    {
        return false;
    }

    struct dirent* entry;
    std::string found_path;

    while((entry = readdir(dir)) != nullptr)
    {
        if(strncmp(entry->d_name, "hidraw", 6) != 0)
        {
            continue;
        }

        std::string uevent_path = "/sys/class/hidraw/" +
                                  std::string(entry->d_name) + "/device/uevent";
        std::ifstream uevent(uevent_path);

        if(!uevent.is_open())
        {
            continue;
        }

        std::string line;
        std::string hid_uniq;
        std::string hid_id;

        while(std::getline(uevent, line))
        {
            if(line.compare(0, 9, "HID_UNIQ=") == 0)
            {
                hid_uniq = line.substr(9);
            }
            else if(line.compare(0, 7, "HID_ID=") == 0)
            {
                hid_id = line.substr(7);
            }
        }

        /*-----------------------------------------------------*\
        | Normalize sysfs HID_UNIQ: strip dashes, lowercase     |
        \*-----------------------------------------------------*/
        std::string uniq_norm;

        for(char c : hid_uniq)
        {
            if(c != '-')
            {
                uniq_norm += (char)tolower(c);
            }
        }

        if(uniq_norm != target_norm)
        {
            continue;
        }

        /*-----------------------------------------------------*\
        | Verify Logitech VID (046D) in HID_ID field             |
        | Format: "0003:0000046D:0000C355"                       |
        \*-----------------------------------------------------*/
        if(hid_id.find("0000046D") == std::string::npos &&
           hid_id.find("0000046d") == std::string::npos)
        {
            continue;
        }

        /*-----------------------------------------------------*\
        | Found a matching hidraw. Use hid_enumerate to find     |
        | the HID++ interface (usage_page 0xFF00) on this PID.  |
        \*-----------------------------------------------------*/
        std::string dev_path = "/dev/" + std::string(entry->d_name);

        LOG_DEBUG("%s Scan: unitId match at %s (pid=0x%s, current=%s)",
                  LOG_TAG, dev_path.c_str(),
                  hid_id.substr(hid_id.rfind(':') + 1).c_str(),
                  location.c_str());

        /*-----------------------------------------------------*\
        | Skip if this is our current path — device hasn't moved |
        \*-----------------------------------------------------*/
        if(dev_path == location)
        {
            LOG_DEBUG("%s Scan: skipping (same path)", LOG_TAG);
            continue;
        }

        /*-----------------------------------------------------*\
        | Extract PID from HID_ID to enumerate the right device. |
        | Format: "0003:0000046D:0000XXXX"                       |
        \*-----------------------------------------------------*/
        size_t last_colon = hid_id.rfind(':');

        if(last_colon == std::string::npos)
        {
            continue;
        }

        unsigned int pid = 0;
        sscanf(hid_id.c_str() + last_colon + 1, "%x", &pid);

        /*-----------------------------------------------------*\
        | Skip if same PID as current path — same receiver,      |
        | just a different pairing slot (stale pairing).         |
        | Genuine connection changes always involve a PID change |
        | (e.g., wireless 0x40B4 → wired 0xC355).               |
        \*-----------------------------------------------------*/
        if(pid == current_pid)
        {
            continue;
        }

        /*-----------------------------------------------------*\
        | Enumerate HID devices with this PID to find the one    |
        | with usage_page 0xFF00 (HID++ interface).              |
        \*-----------------------------------------------------*/
        hid_device_info* devs = hid_enumerate(0x046D, (uint16_t)pid);
        int enum_count = 0;

        for(hid_device_info* d = devs; d != nullptr; d = d->next)
        {
            enum_count++;
            LOG_TRACE("%s Scan: enumerate PID=0x%04X path=%s page=0x%04X usage=%d",
                      LOG_TAG, pid, d->path, d->usage_page, d->usage);

            if(d->usage_page == 0xFF00 && d->usage == 2 &&
               std::string(d->path) != location)
            {
                found_path = d->path;
                break;
            }
        }

        if(enum_count == 0)
        {
            LOG_DEBUG("%s Scan: hid_enumerate(0x046D, 0x%04X) returned no devices", LOG_TAG, pid);
        }

        hid_free_enumeration(devs);

        if(!found_path.empty())
        {
            break;
        }
    }

    closedir(dir);

    if(found_path.empty())
    {
        LOG_DEBUG("%s Scan: no matching device found on alternate path", LOG_TAG);
        return false;
    }

    /*---------------------------------------------------------*\
    | Found the device on a new path with a different PID.      |
    | This is a genuine connection change. Open and migrate.    |
    \*---------------------------------------------------------*/
    hid_device* new_dev = hid_open_path(found_path.c_str());

    if(!new_dev)
    {
        LOG_DEBUG("%s Scan: found %s but failed to open", LOG_TAG, found_path.c_str());
        return false;
    }

    LOG_INFO("%s Device migrated: %s -> %s", LOG_TAG, location.c_str(), found_path.c_str());

    SwapHIDHandle(new_dev, found_path);
    return true;
}

void LogitechHIDPPController::SwapHIDHandle
    (
    hid_device*         new_dev,
    const std::string&  new_path
    )
{
    /*---------------------------------------------------------*\
    | Stop reader/power threads, swap HID handle, restart.       |
    | Called from the power thread — we can't join ourselves,    |
    | so we stop the reader, swap, and flag for restart.         |
    |                                                           |
    | Actually, we're called from the power thread's scan loop.  |
    | The reader thread is running. We need to:                  |
    | 1. Stop the reader thread                                  |
    | 2. Close old handle, set new one                           |
    | 3. Restart reader thread                                   |
    | 4. Reinit device                                           |
    | The power thread keeps running throughout.                 |
    \*---------------------------------------------------------*/

    /*---------------------------------------------------------*\
    | Stop reader thread                                         |
    \*---------------------------------------------------------*/
    reader_running = false;

    if(reader_thread && reader_thread->joinable())
    {
        reader_thread->join();
    }

    delete reader_thread;
    reader_thread = nullptr;

    /*---------------------------------------------------------*\
    | Swap HID handle                                           |
    \*---------------------------------------------------------*/
    hid_close(dev);
    dev       = new_dev;
    location  = new_path;
    log_tag   = "[HID++ " + caps.device_name + "]";

    /*---------------------------------------------------------*\
    | Re-discover transport (might change between Centurion and  |
    | standard HID++ if device switches connection types).       |
    \*---------------------------------------------------------*/
    DiscoverTransport();

    /*---------------------------------------------------------*\
    | Reset state for new connection                            |
    \*---------------------------------------------------------*/
    FlushResponseQueue();
    device_online.store(true);
    consecutive_timeouts.store(0);
    sw_control_claimed = false;
    frame_counter = 0;

    {
        std::lock_guard<std::mutex> lock(power_mutex);
        dim_brightness_pct.store(100);
        power_state = HIDPP_POWER_ACTIVE;
    }

    /*---------------------------------------------------------*\
    | Restart reader thread on new handle                        |
    \*---------------------------------------------------------*/
    reader_running = true;
    reader_thread = new std::thread(&LogitechHIDPPController::ReaderThreadFunc, this);

    /*---------------------------------------------------------*\
    | Reinit device with colors                                 |
    \*---------------------------------------------------------*/
    if(reinit_direct_fn)
    {
        reinit_direct_fn();
    }

    if(caps.has_power_mgmt)
    {
        ReadFirmwareTimers();
        ReadNvSleepRampConfig();
    }

    LOG_INFO("%s Device reconnected on new path — colors restored", LOG_TAG);
}

void LogitechHIDPPController::StartEventWatcher()
{
    /*---------------------------------------------------------*\
    | Start reader thread only (no power thread) to watch for    |
    | connection events on Centurion dongles without sub-devices. |
    | When ConnectionStateChangedEvent arrives, pending_connection|
    | is set for the power thread — but since there's no power   |
    | thread, we need a minimal processing loop.                 |
    \*---------------------------------------------------------*/
    if(reader_running)
    {
        return;
    }

    pending_connection = 0;
    reader_running = true;
    reader_thread = new std::thread(&LogitechHIDPPController::ReaderThreadFunc, this);

    /*---------------------------------------------------------*\
    | Start power thread to process connection events.           |
    | It won't do dim/sleep (no RGB) but it handles              |
    | pending_connection for sub-device re-probe.                |
    \*---------------------------------------------------------*/
    power_thread_running = true;
    power_thread = new std::thread(&LogitechHIDPPController::PowerThreadFunc, this);

    LOG_DEBUG("%s Event watcher started (watching for sub-device)", LOG_TAG);
}

void LogitechHIDPPController::StartPowerManager()
{
    if(caps.idx_rgb_effects == 0 || !caps.has_power_mgmt)
    {
        return;
    }

    if(reader_running)
    {
        return;
    }

    ReadFirmwareTimers();
    ReadNvSleepRampConfig();
    ReadActiveProfileSector();

    /*---------------------------------------------------------*\
    | Don't claim SW control here. The device runs its firmware  |
    | effect (or saved hardware profile) until DeviceUpdateLEDs  |
    | is called for the first time, at which point claim + push  |
    | happen atomically.                                         |
    |                                                           |
    | Reader and power threads still start so we can detect      |
    | migration events (USB plug-in) and process activity events |
    | once SW control is eventually claimed.                     |
    \*---------------------------------------------------------*/
    power_state = HIDPP_POWER_ACTIVE;
    pending_activity = -1;

    reader_running = true;
    reader_thread = new std::thread(&LogitechHIDPPController::ReaderThreadFunc, this);

    power_thread_running = true;
    power_thread = new std::thread(&LogitechHIDPPController::PowerThreadFunc, this);

    LOG_DEBUG("%s Power manager started (idle=%us sleep=%us)",
              LOG_TAG, idle_timeout_s, sleep_timeout_s);
}

void LogitechHIDPPController::StopPowerManager()
{
    if(!reader_running && !power_thread_running)
    {
        return;
    }

    /*---------------------------------------------------------*\
    | Stop power thread first (it may be waiting on the queue)   |
    \*---------------------------------------------------------*/
    power_thread_running = false;
    response_cv.notify_all();

    if(power_thread && power_thread->joinable())
    {
        power_thread->join();
    }

    delete power_thread;
    power_thread = nullptr;

    /*---------------------------------------------------------*\
    | Then stop reader thread                                    |
    \*---------------------------------------------------------*/
    reader_running = false;

    if(reader_thread && reader_thread->joinable())
    {
        reader_thread->join();
    }

    delete reader_thread;
    reader_thread = nullptr;

    /*---------------------------------------------------------*\
    | Wake if we were dimmed/sleeping so Shutdown() can          |
    | cleanly release SW control.                                |
    \*---------------------------------------------------------*/
    if(power_state != HIDPP_POWER_ACTIVE)
    {
        Wake();
    }

    LOG_DEBUG("%s Power manager stopped", LOG_TAG);
}

void LogitechHIDPPController::ReaderThreadFunc()
{
    /*---------------------------------------------------------*\
    | Sole HID reader. NEVER sends commands — that would         |
    | deadlock (we'd wait on our own queue for the response).    |
    | Events are flagged via atomic for the power thread.        |
    \*---------------------------------------------------------*/
    while(reader_running.load())
    {
        uint8_t feat = 0, func = 0;
        uint8_t data[60] = {};
        int result = ReadHIDDirect(&feat, &func, data, sizeof(data), 50);

        if(result < 0)
        {
            /*-------------------------------------------------*\
            | HID read error — device handle is invalid (device  |
            | physically removed). Mark offline and sleep to     |
            | avoid spinning.                                    |
            \*-------------------------------------------------*/
            if(device_online.load())
            {
                LOG_DEBUG("%s HID read error — device removed", LOG_TAG);
                device_online.store(false);
            }

            std::this_thread::sleep_for(100ms);
            continue;
        }

        if(result > 0)
        {
            /*-------------------------------------------------*\
            | Check for firmware events first.                   |
            | Events are flagged for the power thread and NOT    |
            | added to the response queue — they aren't command  |
            | responses and would pollute the queue.             |
            \*-------------------------------------------------*/
            if(caps.idx_rgb_effects != 0 &&
               feat == caps.idx_rgb_effects &&
               (func & 0xF0) == 0x10 &&
               (func & 0x0F) != HIDPP_SW_ID)
            {
                pending_activity.store((int)data[0]);
                continue;
            }

            /*-------------------------------------------------*\
            | CentPPBridge event 0: ConnectionStateChangedEvent  |
            | Sub-device connected or disconnected from dongle.  |
            \*-------------------------------------------------*/
            if(transport.bridge_feat_idx != 0 &&
               feat == transport.bridge_feat_idx &&
               (func & 0xF0) == 0x00 &&
               (func & 0x0F) != HIDPP_SW_ID)
            {
                /*-------------------------------------------------*\
                | ConnectionStateChangedEvent payload:               |
                |   data[0] = ? (always 0 in observed packets)      |
                |   data[1] = number of connected sub-devices       |
                |   data[2] = ?                                     |
                | Connect: [00 01 00], Disconnect: [00 00 00]       |
                \*-------------------------------------------------*/
                uint8_t num_devices = data[1];
                LOG_DEBUG("%s Bridge ConnectionStateChanged: %d sub-device(s) (data: %02X %02X %02X)",
                          LOG_TAG, num_devices, data[0], data[1], data[2]);
                /*-------------------------------------------------*\
                | Store +1 for connected, -1 for disconnected.      |
                | Power thread checks sign to decide action.        |
                \*-------------------------------------------------*/
                pending_connection.store(num_devices > 0 ? 1 : -1);
                continue;
            }

            /*-------------------------------------------------*\
            | Feature 0x1D4B event 0: WirelessStatus             |
            | Device reconnected after power cycle.              |
            | Use cached map lookup only — reader thread must     |
            | never send commands (deadlock risk).                |
            \*-------------------------------------------------*/
            {
                auto it = caps.feature_map.find(0x1D4B);
                uint8_t ws_idx = (it != caps.feature_map.end()) ? it->second : 0;

                if(ws_idx != 0 && feat == ws_idx &&
                   (func & 0xF0) == 0x00 &&
                   (func & 0x0F) != HIDPP_SW_ID)
                {
                    uint8_t reconnect    = data[0];
                    uint8_t config_needed = data[1];

                    LOG_DEBUG("%s WirelessStatus event: reconnect=%d config_needed=%d",
                              LOG_TAG, reconnect, config_needed);

                    /*-------------------------------------------------*\
                    | Push colors on BOTH events:                       |
                    |  reconnect=1: immediately override boot animation |
                    |  config_needed=0: re-push after firmware boot     |
                    |    completes (firmware may overwrite per-key       |
                    |    buffer during its internal initialization)     |
                    \*-------------------------------------------------*/
                    pending_connection.store(1);

                    continue;
                }
            }

            /*-------------------------------------------------*\
            | Only queue responses to OUR commands.              |
            | Our commands use HIDPP_SW_ID (0x0A) in the low     |
            | nibble. Firmware-generated messages (battery,       |
            | sync, etc.) use SW_ID 0 — drop those silently.    |
            | Error responses (feat=0xFF) are always queued.     |
            \*-------------------------------------------------*/
            if(feat != 0xFF && (func & 0x0F) != HIDPP_SW_ID)
            {
                continue;
            }
            {
                std::lock_guard<std::mutex> lock(response_mutex);
                HIDPPRawMessage msg;
                msg.feat   = feat;
                msg.func   = func;
                msg.result = result;
                memcpy(msg.data, data, sizeof(msg.data));
                response_queue.push_back(msg);
            }
            response_cv.notify_all();
        }
    }
}

void LogitechHIDPPController::PowerThreadFunc()
{
    /*---------------------------------------------------------*\
    | Handles power state machine and sends commands.            |
    | Reads responses from the queue (filled by reader thread).  |
    \*---------------------------------------------------------*/
    auto last_probe_time = std::chrono::steady_clock::now();

    while(power_thread_running.load())
    {
        /*-----------------------------------------------------*\
        | Watcher mode: device failed initial probe. Retry       |
        | IRoot every 5 seconds until device becomes reachable.  |
        \*-----------------------------------------------------*/
        if(watcher_mode.load())
        {
            auto now = std::chrono::steady_clock::now();

            if(now - last_probe_time >= std::chrono::seconds(5))
            {
                last_probe_time = now;

                uint8_t test_idx = GetFeatureIndex(HIDPP_FEAT_FEATURE_SET);

                if(test_idx != 0)
                {
                    LOG_INFO("%s Device became reachable — initiating full probe", LOG_TAG);
                    FullReprobe();
                }
            }

            std::this_thread::sleep_for(50ms);
            continue;
        }

        /*-----------------------------------------------------*\
        | 1. Check for pending firmware events                   |
        \*-----------------------------------------------------*/
        int activity = pending_activity.exchange(-1);

        if(activity >= 0)
        {
            std::lock_guard<std::mutex> lock(power_mutex);
            OnUserActivity((uint8_t)activity);
        }

        /*-----------------------------------------------------*\
        | 1b. Check for connection state changes                 |
        \*-----------------------------------------------------*/
        int connection = pending_connection.exchange(0);

        if(connection > 0)
        {
            if(HasBridge())
            {
                ReprobeSubDevice();
            }
            else
            {
                ReconnectDevice();
            }
        }
        else if(connection < 0)
        {
            LOG_DEBUG("%s Device disconnected", LOG_TAG);
            device_online.store(false);
        }

        /*-----------------------------------------------------*\
        | 1c. Continuous scan for connection migration.          |
        | Checks sysfs every 2 seconds for our unitId on a      |
        | hidraw with a DIFFERENT PID. Same-PID matches are     |
        | filtered out (stale receiver pairing slots), so this  |
        | only fires on genuine connection changes (e.g., USB   |
        | plug-in changes PID from 0x40B4 → 0xC355).            |
        \*-----------------------------------------------------*/
        if(!caps.unit_id.empty())
        {
            auto now = std::chrono::steady_clock::now();

            if(now - last_probe_time >= std::chrono::seconds(2))
            {
                last_probe_time = now;
                ScanForDevice();
            }
        }

        /*-----------------------------------------------------*\
        | 2. Power management timing                             |
        \*-----------------------------------------------------*/
        {
            std::lock_guard<std::mutex> lock(power_mutex);

            switch(power_state)
            {
                case HIDPP_POWER_DIMMING:
                {
                    auto now = std::chrono::steady_clock::now();
                    if(now >= next_dim_time)
                    {
                        DimRampStep();
                        next_dim_time = now + std::chrono::milliseconds(DIM_INTERVAL_MS);
                    }
                    break;
                }

                case HIDPP_POWER_IDLE:
                    if(sleep_timeout_s > 0 &&
                       std::chrono::steady_clock::now() >= sleep_deadline)
                    {
                        StartSleep();
                    }
                    break;

                default:
                    break;
            }
        }

        std::this_thread::sleep_for(50ms);
    }
}

void LogitechHIDPPController::FlushResponseQueue()
{
    std::lock_guard<std::mutex> lock(response_mutex);
    size_t flushed = response_queue.size();
    response_queue.clear();

    if(flushed > 0)
    {
        LOG_DEBUG("%s Flushed %zu stale messages from response queue", LOG_TAG, flushed);
    }
}

void LogitechHIDPPController::DispatchEvent
    (
    uint8_t         feat,
    uint8_t         func,
    const uint8_t*  data
    )
{
    if(caps.idx_rgb_effects == 0 || data == nullptr)
    {
        return;
    }

    /*---------------------------------------------------------*\
    | onUserActivity = event 1 on RGB Effects (0x8071)           |
    | Event function byte: (1 << 4) | fw_swid                   |
    | Our commands use HIDPP_SW_ID (0x0A); firmware events use   |
    | a different sw_id (typically 0).                           |
    \*---------------------------------------------------------*/
    if(feat == caps.idx_rgb_effects &&
       (func & 0xF0) == 0x10 &&
       (func & 0x0F) != HIDPP_SW_ID)
    {
        OnUserActivity(data[0]);
    }
}

void LogitechHIDPPController::OnUserActivity(uint8_t activity_type)
{
    /*---------------------------------------------------------*\
    | power_mutex must already be held by the caller.            |
    \*---------------------------------------------------------*/

    if(activity_type == 0)
    {
        /*-----------------------------------------------------*\
        | IDLE event — firmware detected inactivity.             |
        | Only act if we're currently ACTIVE.                    |
        | Firmware sends a burst of ~8 events; ignore repeats.  |
        \*-----------------------------------------------------*/
        if(power_state != HIDPP_POWER_ACTIVE)
        {
            return;
        }

        LOG_DEBUG("%s onUserActivity: IDLE — starting dim", LOG_TAG);

        /*-----------------------------------------------------*\
        | Flush stale per-key ACKs before sending commands       |
        \*-----------------------------------------------------*/
        FlushResponseQueue();

        /*-----------------------------------------------------*\
        | Switch to flags=3 (ZONE|POWER): release EFFECT to|
        | firmware, monitor for user activity (keypress).        |
        \*-----------------------------------------------------*/
        SetSWControl(3, 3);
        StartDimRamp();
    }
    else
    {
        /*-----------------------------------------------------*\
        | ACTIVE event — user resumed typing.                    |
        | Only act if we're NOT already active.                  |
        \*-----------------------------------------------------*/
        if(power_state == HIDPP_POWER_ACTIVE)
        {
            return;
        }

        LOG_DEBUG("%s onUserActivity: ACTIVE — waking", LOG_TAG);
        Wake();
    }
}

void LogitechHIDPPController::WakeIfNeeded()
{
    std::lock_guard<std::mutex> lock(power_mutex);

    if(power_state == HIDPP_POWER_ACTIVE)
    {
        return;
    }

    LOG_DEBUG("%s WakeIfNeeded: waking from state %d", LOG_TAG, power_state);
    Wake();
}

void LogitechHIDPPController::StartDimRamp()
{
    /*---------------------------------------------------------*\
    | Start the brightness ramp from 100% to DIM_TARGET_PCT.    |
    | The actual dimming happens in DeviceUpdateLEDs — it reads  |
    | dim_brightness_pct and scales the color buffer output.    |
    | This is our own host-side animation, independent of the   |
    | firmware's sleep-ramp timer.                               |
    \*---------------------------------------------------------*/
    dim_step      = 0;
    next_dim_time = std::chrono::steady_clock::now();
    power_state   = HIDPP_POWER_DIMMING;

    LOG_DEBUG("%s Dim ramp started (100%% -> %d%%)", LOG_TAG, DIM_TARGET_PCT);
}

void LogitechHIDPPController::DimRampStep()
{
    /*---------------------------------------------------------*\
    | power_mutex must already be held by the caller.            |
    | Adjusts brightness and requests a repaint so              |
    | DeviceUpdateLEDs pushes the dimmed colors.                |
    \*---------------------------------------------------------*/
    if(power_state != HIDPP_POWER_DIMMING)
    {
        return;
    }

    dim_step++;

    int brightness = 100 - ((100 - DIM_TARGET_PCT) * dim_step / DIM_STEPS);

    if(brightness < DIM_TARGET_PCT)
    {
        brightness = DIM_TARGET_PCT;
    }

    dim_brightness_pct.store(brightness);

    /*---------------------------------------------------------*\
    | Request repaint so DeviceUpdateLEDs applies the new        |
    | brightness. For animations this is redundant (the          |
    | animation loop already calls it), but for static colors   |
    | this is the only way to push the dimmed output.           |
    \*---------------------------------------------------------*/
    init_generation++;

    if(request_repaint_fn)
    {
        request_repaint_fn();
    }

    /*---------------------------------------------------------*\
    | Check if dim ramp is complete                              |
    \*---------------------------------------------------------*/
    if(dim_step >= DIM_STEPS)
    {
        power_state = HIDPP_POWER_IDLE;

        /*-----------------------------------------------------*\
        | Pull the sleep deadline forward by the firmware's      |
        | off-ramp duration so the firmware fade *ends* at the   |
        | user-configured sleep_timeout_s. Without this we'd    |
        | be late by nv_sleep_ramp_seconds (30s on G515).       |
        \*-----------------------------------------------------*/
        uint16_t effective_sleep = sleep_timeout_s;

        if(caps.nv_sleep_ramp_known && caps.nv_sleep_ramp_enabled
           && caps.nv_sleep_ramp_seconds < sleep_timeout_s)
        {
            effective_sleep -= caps.nv_sleep_ramp_seconds;
        }

        uint16_t sleep_delay = (effective_sleep > idle_timeout_s)
                             ? (effective_sleep - idle_timeout_s) : 0;

        sleep_deadline = std::chrono::steady_clock::now()
                       + std::chrono::seconds(sleep_delay);

        LOG_DEBUG("%s Dim complete — IDLE (sleep in %us, effective_sleep=%us)",
                  LOG_TAG, sleep_delay, effective_sleep);
    }
}

void LogitechHIDPPController::StartSleep()
{
    /*---------------------------------------------------------*\
    | SetRgbPowerMode(3) = firmware-managed fade to off.         |
    | The firmware handles the fade internally.                  |
    \*---------------------------------------------------------*/
    LOG_DEBUG("%s Entering sleep (SetRgbPowerMode 3)", LOG_TAG);

    uint8_t data[3] = { 0x01, 0x03, 0x00 };
    SendLong(caps.idx_rgb_effects, caps.fn_pwr_mode, data, 3);

    blankFAPmessage response;
    ReadResponse(response, caps.idx_rgb_effects, caps.fn_pwr_mode);

    power_state = HIDPP_POWER_SLEEPING;
}

void LogitechHIDPPController::Wake()
{
    /*---------------------------------------------------------*\
    | Restore from any non-ACTIVE state.                         |
    | power_mutex must already be held by the caller.            |
    \*---------------------------------------------------------*/
    HIDPPPowerState prev = power_state;

    /*---------------------------------------------------------*\
    | Flush stale per-key ACKs from the dim ramp so subsequent   |
    | commands can find their responses.                          |
    \*---------------------------------------------------------*/
    FlushResponseQueue();

    if(prev == HIDPP_POWER_SLEEPING)
    {
        /*-----------------------------------------------------*\
        | The sleep command starts a firmware fade-to-off that   |
        | takes ~30s. During that window, the device is still    |
        | responsive — just cancel with SetRGBPowerMode(1).      |
        |                                                        |
        | If the device has fully gone offline and reconnected,  |
        | it will have reset to onboard profile mode and the     |
        | first command may get no response. In that case we     |
        | need a full re-init (host mode, zone effects, etc.).   |
        \*-----------------------------------------------------*/
        bool quick_wake = false;

        {
            uint8_t data[2] = { 0x01, 0x01 };
            SendLong(caps.idx_rgb_effects, caps.fn_pwr_mode, data, 2);

            blankFAPmessage response;
            int result = ReadResponse(response, caps.idx_rgb_effects, caps.fn_pwr_mode, 500);

            if(result > 0)
            {
                quick_wake = true;
                LOG_DEBUG("%s Wake: device still responsive (fade window)", LOG_TAG);
            }
        }

        if(!quick_wake)
        {
            /*-------------------------------------------------*\
            | Device went fully offline. Retry with delays,      |
            | then do full re-init.                              |
            \*-------------------------------------------------*/
            for(int attempt = 0; attempt < 5; attempt++)
            {
                LOG_DEBUG("%s Wake: device offline, retry %d", LOG_TAG, attempt + 1);
                std::this_thread::sleep_for(200ms);

                uint8_t data[2] = { 0x01, 0x01 };
                SendLong(caps.idx_rgb_effects, caps.fn_pwr_mode, data, 2);

                blankFAPmessage response;
                int result = ReadResponse(response, caps.idx_rgb_effects, caps.fn_pwr_mode, 500);

                if(result > 0)
                {
                    break;
                }
            }

            /*-------------------------------------------------*\
            | Full re-init: host mode (device reset to onboard)  |
            \*-------------------------------------------------*/
            if(caps.idx_profile_management != 0)
            {
                uint8_t data[1] = { 0x05 };
                SendLong(caps.idx_profile_management, FN_8101_GET_SET_MODE, data, 1);

                blankFAPmessage response;
                ReadResponse(response, caps.idx_profile_management, FN_8101_GET_SET_MODE);

                LOG_DEBUG("%s Wake: ProfileManagement set to host mode", LOG_TAG);
            }
            else if(caps.idx_onboard_profiles != 0)
            {
                uint8_t data[1] = { 0x02 };
                SendLong(caps.idx_onboard_profiles, FN_8100_SET_ONBOARD_MODE, data, 1);

                blankFAPmessage response;
                ReadResponse(response, caps.idx_onboard_profiles, FN_8100_SET_ONBOARD_MODE);

                LOG_DEBUG("%s Wake: OnboardProfiles set to host mode", LOG_TAG);
            }
        }
    }

    /*---------------------------------------------------------*\
    | Reset state for reinit. ReinitDirectMode (called via the   |
    | RGBController callback) does the proper sequence:          |
    |   1. Claim SW control (host mode + flags + power mode)     |
    |   2. Zone effect with real color from buffer (via          |
    |      DeviceUpdateLEDs sent_colors-empty path)              |
    |   3. Full per-key push                                     |
    \*---------------------------------------------------------*/
    sw_control_claimed = false;
    dim_brightness_pct.store(100);
    init_generation++;

    power_state = HIDPP_POWER_ACTIVE;

    LOG_DEBUG("%s Woke from state %d", LOG_TAG, prev);

    if(reinit_direct_fn)
    {
        reinit_direct_fn();
    }
}

void LogitechHIDPPController::ReadFirmwareTimers()
{
    if(caps.idx_rgb_effects == 0 || !caps.has_power_mgmt)
    {
        return;
    }

    /*---------------------------------------------------------*\
    | GetRgbPowerModeConfig (fn7, sub-function 0x00 = get)       |
    | Response: [echo], idle_hi, idle_lo, sleep_hi, sleep_lo     |
    \*---------------------------------------------------------*/
    uint8_t send_data[1] = { 0x00 };
    uint8_t recv_data[16] = {};

    int result = SendAndReceive(caps.idx_rgb_effects, caps.fn_pwr_config,
                                send_data, 1, recv_data, sizeof(recv_data));

    if(result > 0)
    {
        uint16_t idle  = ((uint16_t)recv_data[3] << 8) | recv_data[4];
        uint16_t sleep = ((uint16_t)recv_data[5] << 8) | recv_data[6];

        if(idle > 0)
        {
            idle_timeout_s = idle;
        }

        if(sleep > 0)
        {
            sleep_timeout_s = sleep;
        }

        LOG_DEBUG("%s Firmware timers: idle=%us sleep=%us", LOG_TAG, idle_timeout_s, sleep_timeout_s);
    }
    else
    {
        LOG_DEBUG("%s Failed to read firmware timers, using defaults (idle=%us sleep=%us)",
                  LOG_TAG, idle_timeout_s, sleep_timeout_s);
    }
}

void LogitechHIDPPController::ReadNvSleepRampConfig()
{
    /*---------------------------------------------------------*\
    | RGBEffects fn3 NV_CONFIG (0x30) read of capability 0x0020 |
    | (Off Ramp / Sleep Transition).                            |
    |                                                           |
    | Wire format (matches GHUB capture line 901):              |
    |   request:  short msg, data = [0x00, cap_hi, cap_lo]      |
    |             where 0x00 = sub-function GET                 |
    |   response: long msg, data = [echo (3 bytes), enabled,    |
    |             ramp_seconds, ...]                             |
    |                                                           |
    | G515 default observed in GHUB: enabled=0x01, seconds=0x1E |
    | (= 30 seconds dim ramp before sleep).                     |
    \*---------------------------------------------------------*/
    if(caps.idx_rgb_effects == 0 || !caps.has_power_mgmt)
    {
        return;
    }

    uint8_t send_data[3] = { 0x00, 0x00, 0x20 };
    uint8_t recv_data[16] = {};

    int result = SendAndReceive(caps.idx_rgb_effects, FN_8071_NV_CONFIG,
                                send_data, sizeof(send_data),
                                recv_data, sizeof(recv_data));

    if(result <= 0)
    {
        LOG_DEBUG("%s NvConfig 0x0020 read failed (result=%d)", LOG_TAG, result);
        return;
    }

    if(recv_data[0] != 0x00 || recv_data[1] != 0x00 || recv_data[2] != 0x20)
    {
        LOG_DEBUG("%s NvConfig 0x0020 read: unexpected echo %02X %02X %02X",
                  LOG_TAG, recv_data[0], recv_data[1], recv_data[2]);
        return;
    }

    caps.nv_sleep_ramp_enabled = (recv_data[3] != 0);
    caps.nv_sleep_ramp_seconds = recv_data[4];
    caps.nv_sleep_ramp_known   = true;

    LOG_DEBUG("%s NvConfig 0x0020 (sleep ramp): enabled=%d ramp=%us "
              "raw=[%02X %02X %02X %02X %02X %02X %02X %02X]",
              LOG_TAG,
              (int)caps.nv_sleep_ramp_enabled,
              (unsigned)caps.nv_sleep_ramp_seconds,
              recv_data[3], recv_data[4], recv_data[5], recv_data[6],
              recv_data[7], recv_data[8], recv_data[9], recv_data[10]);
}

void LogitechHIDPPController::WritePowerConfig(uint16_t idle_s, uint16_t sleep_s)
{
    /*---------------------------------------------------------*\
    | SetRgbPowerModeConfig (fn7, sub-function 0x01 = set)       |
    | Wire format (long message, 16 bytes payload, matches the  |
    | GET response layout at the same offsets):                 |
    |   [0x01, 0x00, 0x00, idle_hi, idle_lo, sleep_hi, sleep_lo, |
    |    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]  |
    |                                                           |
    | These are the firmware's *runtime* power timers — the     |
    | values reset on power cycle but persist across SW control |
    | release/reclaim, so we need to write them ourselves on    |
    | every claim to be safe.                                    |
    \*---------------------------------------------------------*/
    if(caps.idx_rgb_effects == 0 || !caps.has_power_mgmt)
    {
        return;
    }

    uint8_t data[16] = {};
    data[0] = 0x01;     // sub-function: SET
    data[3] = (uint8_t)((idle_s  >> 8) & 0xFF);
    data[4] = (uint8_t)( idle_s        & 0xFF);
    data[5] = (uint8_t)((sleep_s >> 8) & 0xFF);
    data[6] = (uint8_t)( sleep_s       & 0xFF);

    SendLong(caps.idx_rgb_effects, caps.fn_pwr_config, data, sizeof(data));

    blankFAPmessage response;
    ReadResponse(response, caps.idx_rgb_effects, caps.fn_pwr_config);

    LOG_DEBUG("%s WritePowerConfig: idle=%us sleep=%us", LOG_TAG, idle_s, sleep_s);
}

void LogitechHIDPPController::ReadActiveProfileSector()
{
    /*---------------------------------------------------------*\
    | Diagnostic-only read of the active profile sector via     |
    | ProfileManagement (0x8101) load + paged readBuffer.        |
    |                                                           |
    | This sector is the canonical storage for persisted device |
    | state on G-series devices: idle/sleep timers, baseline    |
    | RGB effect, FKC enable, and more. The HID++ feature       |
    | endpoints (0x8071, 0x8081, 0x1B05, ...) are mostly status |
    | hooks; the configuration database lives here. We don't    |
    | act on the contents — just log them so we can see what    |
    | the device thinks its persisted state is.                  |
    |                                                           |
    | Wire format mirrors GHUB capture lines 670 (load) and     |
    | 672-685 (7× readBuffer):                                   |
    |   load:        long msg, [partition=0x01, sector=0x01,    |
    |                           size_hi=0x00, size_lo=0x63,     |
    |                           padding to 16 bytes]            |
    |   readBuffer:  short msg, [offset_hi, offset_lo, 0]       |
    |                returns long msg with 16 bytes of data     |
    |                                                           |
    | Sector size 0x63 = 99 bytes is what GHUB requested for    |
    | the G515 active profile. Other devices may differ — we    |
    | hardcode it for now since this is diagnostic-only.        |
    \*---------------------------------------------------------*/
    if(caps.idx_profile_management == 0)
    {
        return;
    }

    constexpr uint16_t SECTOR_SIZE = 0x63;  // 99 bytes
    constexpr uint16_t PAGE_SIZE   = 16;

    /*---------------------------------------------------------*\
    | Step 1: load the sector into the device's read buffer     |
    \*---------------------------------------------------------*/
    uint8_t load_data[16] = {};
    load_data[0] = 0x01;                                    // partition: NVS/flash
    load_data[1] = 0x01;                                    // sector: active profile
    load_data[2] = (uint8_t)((SECTOR_SIZE >> 8) & 0xFF);    // size hi
    load_data[3] = (uint8_t)( SECTOR_SIZE       & 0xFF);    // size lo

    SendLong(caps.idx_profile_management, FN_8101_LOAD, load_data, sizeof(load_data));

    blankFAPmessage load_resp;
    int load_result = ReadResponse(load_resp, caps.idx_profile_management, FN_8101_LOAD);

    if(load_result <= 0)
    {
        LOG_DEBUG("%s ProfileSector load failed (result=%d)", LOG_TAG, load_result);
        return;
    }

    /*---------------------------------------------------------*\
    | Step 2: page the sector out 16 bytes at a time            |
    \*---------------------------------------------------------*/
    uint8_t sector_buf[SECTOR_SIZE] = {};

    for(uint16_t offset = 0; offset < SECTOR_SIZE; offset += PAGE_SIZE)
    {
        uint8_t read_req[3] = {
            (uint8_t)((offset >> 8) & 0xFF),
            (uint8_t)( offset       & 0xFF),
            0x00
        };
        uint8_t page_resp[20] = {};

        int result = SendAndReceive(caps.idx_profile_management, FN_8101_READBUFFER,
                                    read_req, sizeof(read_req),
                                    page_resp, sizeof(page_resp));

        if(result <= 0)
        {
            LOG_DEBUG("%s ProfileSector readBuffer offset=0x%04X failed (result=%d)",
                      LOG_TAG, (unsigned)offset, result);
            return;
        }

        size_t copy_len = (offset + PAGE_SIZE > SECTOR_SIZE)
                        ? (size_t)(SECTOR_SIZE - offset)
                        : PAGE_SIZE;
        memcpy(sector_buf + offset, page_resp, copy_len);
    }

    /*---------------------------------------------------------*\
    | Step 3: log as a hexdump, one row per 16 bytes             |
    \*---------------------------------------------------------*/
    LOG_DEBUG("%s ProfileSector partition=NVS sector=1 size=%u bytes:",
              LOG_TAG, (unsigned)SECTOR_SIZE);

    for(uint16_t row = 0; row < SECTOR_SIZE; row += PAGE_SIZE)
    {
        size_t row_len = (row + PAGE_SIZE > SECTOR_SIZE)
                       ? (size_t)(SECTOR_SIZE - row)
                       : PAGE_SIZE;

        char hex[64] = {};
        char* p = hex;
        for(size_t i = 0; i < row_len; i++)
        {
            snprintf(p, 4, "%02X ", sector_buf[row + i]);
            p += 3;
        }

        LOG_DEBUG("%s   %04X: %s", LOG_TAG, (unsigned)row, hex);
    }
}

