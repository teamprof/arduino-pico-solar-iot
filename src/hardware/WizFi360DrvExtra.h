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

#include "../lib/WizFi360/utility/WizFi360Drv.h"
#include "./PinOutput.h"

class WizFi360DrvExtra : public WizFi360Drv
{
public:
    enum SleepMode : int
    {
        DisableSleepMode = 0,
        LightSleepMode = 1,
        ModemSleepMode = 2 // factory default
    };

    WizFi360DrvExtra(uint8_t pinReset, uint8_t pinWakeup);
    void hardwareReset(void);

    bool standby(uint16_t sleepTime = 60000); // deep sleep
    bool wakeup(void);

    bool checkSntpTime(char *timeBuf, size_t bufSize);
    bool getSntpServer(char *buffer, size_t bufSize);

    bool setSleepMode(int sleepMode);
    bool getSleepMode(int *sleepMode);

    bool configAzure(const char *iothubName, const char *deviceID, const char *deviceKey, const char *publishTopic, const char *subscribeTopic);
    bool connectAzure(void);

    bool mqttPublish(const char *message);
    bool mqttDisconnect(void);

private:
    PinOutput pinReset;
    PinOutput pinWakeup;
};
