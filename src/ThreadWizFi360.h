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
#pragma once

#include <mbed.h>
#include <map>
#include <platform/CircularBuffer.h>
#include <ArduinoJson.h>
#include "ThreadBase.h"
#include "./utils/BackoffAlgorithm.h"
#include "./hardware/PinOutput.h"
#include "./hardware/SerialExtra.h"
#include "./hardware/WizFi360DrvExtra.h"
#include "./model/BatteryPacket.h"

#define JsonDocSize 512

class ThreadWizFi360 final : public ThreadBase
{
public:
    ThreadWizFi360(uint32_t queueSize = DefaultQueueSize);

    virtual void onMessage(const Message msg) override;

protected:
    typedef enum _EnumWizfiState : int16_t
    {
        Undefine,
        Standby,
        Wakeup,
        WifiConnect,
        AzureConfigure,
        AzureConnect,
        AzurePublish,
        MqttDisconnect,
        CommandDoneSyncSNTP,
        CommandDonePublishMQTT,
        FatalError
    } EnumWizfiState;
    typedef EnumWizfiState WizfiState;
    WizfiState wizfiState;

    void setWizfiState(WizfiState state);
    const char *getWizfiStateString(WizfiState state);

    BackoffAlgorithm backoffAlgoCmd;

    virtual void setup(void) override;

    bool isSyncRTC;
    void initRTC(void);
    void updateRTC(struct tm &tm);
    const char *timeToText(time_t timestamp);

    solariot::SerialExtra uartWifi;
    WizFi360DrvExtra wizFi360DrvExtra;
    bool resetWifzi360(void);

    char txJsonString[JsonDocSize];
    StaticJsonDocument<JsonDocSize> jsonMessage;
    bool buildTxJsonString(time_t localTime, uint16_t batteryLevel);

    ///////////////////////////////////////////////////////////////////////
    // event handler
    typedef void (ThreadWizFi360::*funcPtr)(const Message &);
    std::map<short, funcPtr> handlerMap;
    __EVENT_FUNC_DECLARATION(EventUartRx)
    __EVENT_FUNC_DECLARATION(EventBatteryStatusUpdate)
    __EVENT_FUNC_DECLARATION(EventWizfiPublishMQTT)
    __EVENT_FUNC_DECLARATION(EventWizfiSyncSNTP)
    __EVENT_FUNC_DECLARATION(EventWizfiStandby)
    ///////////////////////////////////////////////////////////////////////
};
