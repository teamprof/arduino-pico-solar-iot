/*
 * Copyright (C) 2022 teamprof.net@gmail.com or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifdef __cplusplus
extern "C"
{
#endif
#include "hardware/rtc.h"
#include "pico/util/datetime.h"
#ifdef __cplusplus
}
#endif

#include <mbed.h>
#include <hardware/sync.h>
#include <rtc_api.h>
#include "../AppLog.h"
#include "../AppEvent.h"
#include "./ThreadWizFi360.h"
#include "./lib/WizFi360/WizFi360.h"
#include "./lib/base64/base64.hpp"
#include "./hardware/WizFi360DrvExtra.h"
#include "../Credential.h"

using namespace std::chrono_literals;

extern "C" char *strptime(const char *__restrict,
                          const char *__restrict,
                          struct tm *__restrict);

static datetime_t DefaultTimeRTC = {
    .year = 2022,
    .month = 7,
    .day = 1,
    .dotw = 5, // 0 is Sunday, so 5 is Friday
    .hour = 0,
    .min = 0,
    .sec = 0};

#define PinWifiWakeup (21u)
#define PinWifiReset (20u)
#define PinWifiU1Tx (4u)
#define PinWifiU1Rx (5u)
#define PinWifiU1Cts (-1)
#define PinWifiU1Rts (-1)

#define CMD_RETRY_MAX_ATTEMPTS (10)          // max number of retry for Tx
#define CMD_RETRY_MAX_BACKOFF_DELAY_S (300U) // the max back-off delay between retries for Tx (in seconds)
#define CMD_RETRY_BACKOFF_BASE_S (10U)       // the base back-off delay of retry configuration for Tx (in seconds)

#define TimeTextBufferSize 32

#define IntervalSyncSTNP 86400s // sync RTC to STNP every day
// #define IntervalSyncSTNP 60s // 60s for testing only

#define IntervalAzurePublish 5000 // time between two Azure publish, in ms

static WiFiClient client;

ThreadWizFi360::ThreadWizFi360(uint32_t queueSize) : ThreadBase(queueSize),
                                                     uartWifi(PinWifiU1Tx, PinWifiU1Rx),
                                                     wizFi360DrvExtra(PinWifiReset, PinWifiWakeup),
                                                     backoffAlgoCmd((uint16_t)micros()),
                                                     jsonMessage(),
                                                     handlerMap()
{
    isSyncRTC = false;
    wizfiState = Undefine;
    memset(txJsonString, 0, sizeof(txJsonString));

    backoffAlgoCmd.Init(CMD_RETRY_BACKOFF_BASE_S, CMD_RETRY_MAX_BACKOFF_DELAY_S, CMD_RETRY_MAX_ATTEMPTS);

    handlerMap = {
        __EVENT_MAP(ThreadWizFi360, EventUartRx),
        __EVENT_MAP(ThreadWizFi360, EventBatteryStatusUpdate),
        __EVENT_MAP(ThreadWizFi360, EventWizfiPublishMQTT),
        __EVENT_MAP(ThreadWizFi360, EventWizfiSyncSNTP),
        __EVENT_MAP(ThreadWizFi360, EventWizfiStandby),
    };
}

///////////////////////////////////////////////////////////////////////
// event handler
///////////////////////////////////////////////////////////////////////
__EVENT_FUNC_DEFINITION(ThreadWizFi360, EventUartRx, msg)
{
    // LOG_INFO("EventUartRx(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);

    int bytes = uartWifi.available();
    if (bytes <= 0)
    {
        return;
    }
    // LOG_INFO("uartWifi.available() returns ", bytes);

    static constexpr char ResponseExitStandby[] = "ready\r\n";
    static constexpr int RxBufSize = 128;
    static uint8_t rxBuf[RxBufSize + 1]; // +1 for null terminator
    static uint16_t rxIdx = 0;
    if (wizfiState == Standby)
    {
        while (uartWifi.available())
        {
            uint8_t data = uartWifi.read();
            // LOG_INFO("data = ", (char)data, " (", data, ")");
            rxBuf[rxIdx++] = data;
            if (data == '\0')
            {
                continue; // skip null terminator
            }
            else if (data == '\n')
            {
                rxBuf[rxIdx] = '\0';
                if (strncmp((const char *)rxBuf, ResponseExitStandby, sizeof(ResponseExitStandby) - 1) == 0)
                {
                    LOG_INFO("wizFi360 standby timeout");
                    setWizfiState(Undefine); // WizFi360 exits from standby/deep sleep
                    postEvent(EventWizfiStandby);
                    rxIdx = 0;
                }
                else
                {
                    // ignore unknown response
                    // LOG_INFO("ignore rxBuf: ", (const char *)rxBuf);
                    rxIdx = 0;
                }
            }

            if (rxIdx >= RxBufSize)
            {
                LOG_ERROR("rxBuf overflow: ", DebugLogBase::HEX, LOG_AS_ARR(rxBuf, rxIdx));
                rxIdx = 0;
                break;
            }
        }
    }
}

__EVENT_FUNC_DEFINITION(ThreadWizFi360, EventBatteryStatusUpdate, msg)
{
    LOG_INFO("EventBatteryStatusUpdate(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);

    BatteryPacketFactory::produce(time(nullptr), msg.iParam);
    queue.call_in(std::chrono::seconds(1), [this]()
                  { postEvent(EventWizfiPublishMQTT); });
}

__EVENT_FUNC_DEFINITION(ThreadWizFi360, EventWizfiSyncSNTP, msg)
{
    LOG_INFO("EventWizfiSyncSNTP(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);

    static constexpr char NullTime[] = "Thu Jan 01 00:00:00 1970";
    char timeBuf[TimeTextBufferSize];
    struct tm tm;
    while (true)
    {
        switch (wizfiState)
        {
        case Undefine:
        case Standby:
            LOG_DEBUG("wizfiState=", getWizfiStateString(wizfiState), ", call wizFi360DrvExtra.wakeup()");
            if (wizFi360DrvExtra.wakeup())
            {
                setWizfiState(Wakeup);
                continue;
            }
            else
            {
                LOG_WARN("wizFi360DrvExtra.wakeup() failed");
            }
            break;

        case Wakeup:
            LOG_DEBUG("wizfiState=", getWizfiStateString(wizfiState), ", call WiFi.begin()");
            if (WiFi.begin(credential::NetworkSsid, credential::NetworkPassword) == WL_CONNECTED)
            {
                LOG_DEBUG("SSID=", WiFi.SSID(), ", Local IP=", WiFi.localIP(), ", RSSI=", WiFi.RSSI(), " dBm");
                setWizfiState(WifiConnect);
                continue;
            }
            else
            {
                LOG_WARN("WiFi.begin(", credential::NetworkSsid, ", ", credential::NetworkPassword, ") failed");
            }
            break;

        case WifiConnect:
            LOG_DEBUG("wizfiState=", getWizfiStateString(wizfiState), ", call checkSntpTime()");
            if (wizFi360DrvExtra.checkSntpTime(timeBuf, sizeof(timeBuf)) &&
                (strncmp(timeBuf, NullTime, strlen(NullTime)) != 0) &&
                (strptime(timeBuf, "%a %b %d %T %Y", &tm) != NULL))
            // (strptime(timeBuf, "%a %b %d %H:%M:%S %Y", &tm) != NULL))
            {
                // LOG_DEBUG("SNTP returns ", timeBuf);
                // LOG_DEBUG("struct tm: ", tm.tm_year, "-", tm.tm_mon, "-", tm.tm_mday, "(", tm.tm_wday, ") ", tm.tm_hour, ":", tm.tm_min, ":", tm.tm_sec);
                updateRTC(tm);
                setWizfiState(CommandDoneSyncSNTP);
                if (!isSyncRTC)
                {
                    queue.call_every(IntervalSyncSTNP, [this]()
                                     { postEvent(EventWizfiSyncSNTP); });
                }
                isSyncRTC = true;
                continue;
            }
            break;

        case CommandDoneSyncSNTP:
            LOG_DEBUG("wizfiState=", getWizfiStateString(wizfiState), ", postEvent(EventWizfiStandby)");
            setWizfiState(Wakeup);
            postEvent(EventWizfiStandby);
            return;

        default:
            LOG_WARN("Unsupported wizfiState: ", getWizfiStateString(wizfiState));
            return;
        }

        /////////////////////////////////////////////////////////////////////////
        // prepare to retry when the called function returns fail
        /////////////////////////////////////////////////////////////////////////
        uint16_t nextBackoff = 0;
        BackoffAlgorithmStatus_t retryStatus = backoffAlgoCmd.GetNext(&nextBackoff);
        if (retryStatus == BackoffAlgorithmRetriesExhausted)
        {
            // retry sending command expired, reset WizFi360
            LOG_ERROR("Too many failed in EventWizfiSyncSNTP. Reset WizFi360 to standby mode.");
            resetWifzi360();
            if (!isSyncRTC)
            {
                postEvent(EventWizfiSyncSNTP);
            }
            return;
        }

        // note: retryStatus is BackoffAlgorithmSuccess
        LOG_DEBUG("BackoffAlgorithmSuccess: Retry command after ", nextBackoff, "s");
        rtos::ThisThread::sleep_for(nextBackoff * 1000);
    }
}

__EVENT_FUNC_DEFINITION(ThreadWizFi360, EventWizfiPublishMQTT, msg)
{
    LOG_INFO("EventWizfiPublishMQTT(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);
    BatteryPacket packet;
    while (true)
    {
        LOG_DEBUG("wizfiState=", getWizfiStateString(wizfiState));
        switch (wizfiState)
        {
        case Undefine:
        case Standby:
            if (wizFi360DrvExtra.wakeup())
            {
                setWizfiState(Wakeup);
                continue;
            }
            else
            {
                LOG_WARN("wizFi360DrvExtra.wakeup() failed");
            }
            break;

        case Wakeup:
            if ((strlen(txJsonString) > 0) ||
                ((BatteryPacketFactory::consume(packet) && buildTxJsonString(packet.localTime, packet.batteryPercentage))))
            {
                if (WiFi.begin(credential::NetworkSsid, credential::NetworkPassword) == WL_CONNECTED)
                {
                    LOG_DEBUG("SSID=", WiFi.SSID(), ", Local IP=", WiFi.localIP(), ", RSSI=", WiFi.RSSI(), " dBm");
                    setWizfiState(WifiConnect);
                    continue;
                }
                else
                {
                    LOG_WARN("WiFi.begin(", credential::NetworkSsid, ", ", credential::NetworkPassword, ") failed");
                }
            }
            else if (wizFi360DrvExtra.standby())
            {
                setWizfiState(Standby);
                return;
            }
            else
            {
                LOG_WARN("wizFi360DrvExtra.standby() failed");
            }
            break;

        case WifiConnect:
            if (wizFi360DrvExtra.configAzure(credential::IotHubName, credential::DeviceID, credential::DeviceKey, credential::PublishTopic, credential::SubscribeTopic))
            {
                setWizfiState(AzureConfigure);
                continue;
            }
            else
            {
                LOG_WARN("wizFi360DrvExtra.configAzure() failed");
            }
            break;

        case AzureConfigure:
            if (wizFi360DrvExtra.connectAzure())
            {
                setWizfiState(AzureConnect);
                continue;
            }
            else
            {
                LOG_WARN("wizFi360DrvExtra.connectAzure() failed");
            }
            break;

        case AzureConnect:
            ASSERT(strlen(txJsonString) > 0);
            if (wizFi360DrvExtra.mqttPublish(txJsonString))
            {
                LOG_INFO("published: ", txJsonString);
                txJsonString[0] = '\0';
                rtos::ThisThread::sleep_for(IntervalAzurePublish);
                setWizfiState(AzurePublish);
                continue;
            }
            else
            {
                LOG_WARN("wizFi360DrvExtra.mqttPublish() failed");
            }
            break;

        case AzurePublish:
            if (BatteryPacketFactory::consume(packet) && buildTxJsonString(packet.localTime, packet.batteryPercentage))
            {
                setWizfiState(AzureConnect);
                continue;
            }
            else if (wizFi360DrvExtra.mqttDisconnect())
            {
                setWizfiState(CommandDonePublishMQTT);
                continue;
            }
            else
            {
                LOG_WARN("wizFi360DrvExtra.mqttDisconnect() failed");
            }
            break;

        case CommandDonePublishMQTT:
            setWizfiState(Wakeup);
            postEvent(EventWizfiStandby);
            return;

        default:
            LOG_WARN("Unsupported wizfiState: ", getWizfiStateString(wizfiState));
            return;
        }

        /////////////////////////////////////////////////////////////////////////
        // prepare to retry when the called function returns fail
        /////////////////////////////////////////////////////////////////////////
        uint16_t nextBackoff = 0;
        BackoffAlgorithmStatus_t retryStatus = backoffAlgoCmd.GetNext(&nextBackoff);
        if (retryStatus == BackoffAlgorithmRetriesExhausted)
        {
            // retry sending command expired, reset WizFi360
            LOG_ERROR("Too many failed in EventWizfiPublishMQTT. Reset WizFi360 to standby mode.");
            resetWifzi360();
            return;
        }

        // note: retryStatus is BackoffAlgorithmSuccess
        LOG_DEBUG("BackoffAlgorithmSuccess: Retry command after ", nextBackoff, "s");
        rtos::ThisThread::sleep_for(nextBackoff * 1000);
    }
}

__EVENT_FUNC_DEFINITION(ThreadWizFi360, EventWizfiStandby, msg)
{
    LOG_INFO("EventWizfiStandby(", msg.event, "), iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);

    backoffAlgoCmd.Init(CMD_RETRY_BACKOFF_BASE_S, CMD_RETRY_MAX_BACKOFF_DELAY_S, CMD_RETRY_MAX_ATTEMPTS);
    while (true)
    {
        if ((strlen(txJsonString) > 0) || (!BatteryPacketFactory::isEmpty()))
        {
            LOG_INFO("ignore EventWizfiStandby");
            return;
        }
        else if (wizfiState == Standby)
        {
            // nothing to do if it is already in standby state
            LOG_INFO("nothing to do as it is already in standby state");
            return;
        }
        else if (wizFi360DrvExtra.standby())
        {
            setWizfiState(Standby);
            return;
        }
        else
        {
            LOG_WARN("wizFi360DrvExtra.standby() failed");
        }

        uint16_t nextBackoff = 0;
        BackoffAlgorithmStatus_t retryStatus = backoffAlgoCmd.GetNext(&nextBackoff);
        if (retryStatus == BackoffAlgorithmRetriesExhausted)
        {
            // retry sending command expired, reset WizFi360
            LOG_WARN("Too many failed on sending wizFi360DrvExtra.standby(). Reset WizFi360 to standby mode.");
            resetWifzi360();
            return;
        }

        // note: retryStatus is BackoffAlgorithmSuccess
        LOG_DEBUG("BackoffAlgorithmSuccess: Retry wizFi360DrvExtra.standby() after ", nextBackoff, "s");
        rtos::ThisThread::sleep_for(nextBackoff * 1000);
    }
}

void ThreadWizFi360::onMessage(const Message msg)
{
    auto func = handlerMap[msg.event];
    if (func)
    {
        (this->*func)(msg);
    }
    else
    {
        LOG_DEBUG("Unsupported event=", msg.event, ", iParam=", msg.iParam, ", uParam=", msg.uParam, ", lParam=", msg.lParam);
    }
}

const char *ThreadWizFi360::timeToText(time_t timestamp)
{
    static char timeText[TimeTextBufferSize];
    struct tm *tm = localtime(&timestamp);
    size_t size = strftime(timeText, sizeof(timeText), "%y-%m-%d %H:%M:%S", tm);
    // LOG_DEBUG("size = ", size, ", timeBuf = ", timeText);
    return timeText;
}

bool ThreadWizFi360::buildTxJsonString(time_t timestamp, uint16_t batteryLevel)
{
    jsonMessage.clear();
    jsonMessage["deviceId"] = credential::DeviceID;
    jsonMessage["time"] = timeToText(timestamp);
    // jsonMessage["time"] = ctime(&timestamp);
    jsonMessage["batteryPercentage"] = batteryLevel;
    txJsonString[0] = '\0';
    size_t sizeJson = serializeJson(jsonMessage, txJsonString, sizeof(txJsonString));
    if (sizeJson > 0)
    {
        // LOG_INFO("txJsonString=", txJsonString);
    }
    else
    {
        txJsonString[0] = '\0';
        LOG_ERROR("jsonMessage.serialize failed. localTime=", ctime(&timestamp), ", batteryPercentage=", batteryLevel);
    }
}

void ThreadWizFi360::setup(void)
{
    LOG_DEBUG("on core", get_core_num());

    uartWifi.begin(115200);
    uartWifi.onRxEvent([this]()
                       { postEvent(EventUartRx); });

    wizFi360DrvExtra.hardwareReset();
    WiFi.init(&uartWifi);
    if (wizFi360DrvExtra.wakeup())
    {
        setWizfiState(Wakeup);
    }
    else
    {
        setWizfiState(FatalError);
        LOG_ERROR("Fatal error: wizFi360DrvExtra.wakeup() returns false");
        return;
    }

    // if (!wizFi360DrvExtra.setSleepMode(WizFi360DrvExtra::LightSleepMode))
    // {
    //     LOG_ERROR("wizFi360DrvExtra.setSleepMode(LightSleepMode) failed");
    // }

    int sleepMode = -1;
    if (wizFi360DrvExtra.getSleepMode(&sleepMode))
    {
        LOG_DEBUG("wizFi360DrvExtra.getSleepMode()=", sleepMode);
    }
    else
    {
        LOG_ERROR("Fatal error: wizFi360DrvExtra.getSleepMode() returns false");
        return;
    }

    // check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD)
    {
        LOG_ERROR("WiFi shield not present");
        wizfiState = FatalError;
        return;
    }

    initRTC();
    postEvent(EventWizfiSyncSNTP);
}

bool ThreadWizFi360::resetWifzi360(void)
{
    wizFi360DrvExtra.hardwareReset();
    WizFi360Drv::reset();
    if (wizFi360DrvExtra.standby())
    {
        setWizfiState(Standby);
        return true;
    }
    else
    {
        LOG_ERROR("Reset WizFi360 to standby failed!");
        setWizfiState(FatalError);
    }
    return false;
}

void ThreadWizFi360::initRTC(void)
{
    rtc_init();
    rtc_set_datetime(&DefaultTimeRTC);
}

void ThreadWizFi360::updateRTC(struct tm &tm)
{
    datetime_t sntpTime = {
        .year = tm.tm_year + 1900,
        .month = tm.tm_mon + 1,
        .day = tm.tm_mday,
        .dotw = tm.tm_wday, // 0 is Sunday, so 5 is Friday
        .hour = tm.tm_hour,
        .min = tm.tm_min,
        .sec = tm.tm_sec};
    // LOG_DEBUG("datetime_t: ", t.year, "-", t.month, "-", t.day, "(", t.dotw, ") ", t.hour, ":", t.min, ":", t.sec);

    datetime_t rtcTime;
    rtc_get_datetime(&rtcTime);
    if (memcmp(&sntpTime, &rtcTime, sizeof(rtcTime)) != 0)
    {
        LOG_INFO("set RTC to SNTP time");
        rtc_set_datetime(&sntpTime);

        time_t now = time(nullptr);
        const char *timeText = timeToText(now);

        char timeBuf[TimeTextBufferSize];
        datetime_t t;
        rtc_get_datetime(&t);
        datetime_to_str(timeBuf, sizeof(timeBuf), &t);
        LOG_INFO("time()=", timeText, ", datetime_to_str(): ", timeBuf);
    }
    else
    {
        LOG_INFO("RTC and SNTP are same");
    }
}

void ThreadWizFi360::setWizfiState(WizfiState state)
{
    // LOG_INFO(getWizfiStateString(state), "(", state, ")");
    wizfiState = state;

    //  reset command retry counter
    backoffAlgoCmd.Init(CMD_RETRY_BACKOFF_BASE_S, CMD_RETRY_MAX_BACKOFF_DELAY_S, CMD_RETRY_MAX_ATTEMPTS);
}

const char *ThreadWizFi360::getWizfiStateString(WizfiState state)
{
    static char buf[32];
    switch (state)
    {
    case Undefine:
        return "Undefine";
    case Standby:
        return "Standby";
    case Wakeup:
        return "Wakeup";
    case WifiConnect:
        return "WifiConnect";
    case AzureConfigure:
        return "AzureConfigure";
    case AzureConnect:
        return "AzureConnect";
    case AzurePublish:
        return "AzurePublish";
    case MqttDisconnect:
        return "MqttDisconnect";
    case CommandDoneSyncSNTP:
        return "CommandDoneSyncSNTP";
    case CommandDonePublishMQTT:
        return "CommandDonePublishMQTT";
    case FatalError:
        return "FatalError";
    default:
        sprintf(buf, "unknown state: %d", state);
        return buf;
    }
}
