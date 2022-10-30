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
#include "./WizFi360DrvExtra.h"
#include "../../AppLog.h"

// #define NUMWIZFI360TAGS 6

// const char *WIZFI360TAGS[] =
// 	{
// 		"\r\nOK\r\n",
// 		"\r\nERROR\r\n",
// 		"\r\nFAIL\r\n",
// 		"\r\nSEND OK\r\n",
// 		" CONNECT\r\n",
// 		"CLOSED\r\n"};

typedef enum
{
    TAG_OK,
    TAG_ERROR,
    TAG_FAIL,
    TAG_SENDOK,
    TAG_CONNECT,
    TAG_CLOSE
} TagsEnum;

WizFi360DrvExtra::WizFi360DrvExtra(uint8_t pinReset, uint8_t pinWakeup) : WizFi360Drv(),
                                                                          pinReset(pinReset, HIGH),
                                                                          pinWakeup(pinWakeup, HIGH)
{
}

void WizFi360DrvExtra::hardwareReset(void)
{
    pinReset.write(LOW);
    delay(1);
    pinReset.write(HIGH);
    delay(500);
}

bool WizFi360DrvExtra::standby(uint16_t sleepTime)
{
    pinWakeup.write(LOW);
    int ret = sendCmd(F("AT+GSLP=%u"), 1000, sleepTime);
    return (ret == TAG_OK);
}

bool WizFi360DrvExtra::wakeup(void)
{
    pinWakeup.write(HIGH);
    delay(500);
    // delay(1000);
    WizFi360Drv::reset();
    int ret = sendCmd(F("AT"));
    return (ret == TAG_OK);
}

bool WizFi360DrvExtra::setSleepMode(int sleepMode)
{
    int ret = sendCmd(F("AT+SLEEP=%d"), 1000, sleepMode);
    return (ret == TAG_OK);
}

bool WizFi360DrvExtra::getSleepMode(int *sleepMode)
{
    char buf[10];
    if (!sendCmdGet(F("AT+SLEEP?"), F("+SLEEP:"), F("\r\n"), buf, sizeof(buf)))
        return false;

    *sleepMode = atoi(buf);
    return true;
}

bool WizFi360DrvExtra::checkSntpTime(char *timeBuf, size_t bufSize)
{
    timeBuf[0] = '\0';
    if (!sendCmdGet(F("AT+CIPSNTPTIME?"), F("+CIPSNTPTIME:"), F("\r\n"), timeBuf, bufSize))
        return false;

    return true;
}
bool WizFi360DrvExtra::getSntpServer(char *buffer, size_t bufSize)
{
    buffer[0] = '\0';
    if (!sendCmdGet(F("AT+CIPSNTPCFG?"), F("+CIPSNTPCFG:"), F("\r\n"), buffer, bufSize))
        return false;

    return true;
}

bool WizFi360DrvExtra::configAzure(const char *iothubName, const char *deviceID, const char *deviceKey, const char *publishTopic, const char *subscribeTopic)
{
    //  AT+AZSET="iothub_name","device_id","device_key"
    //  OK
    if (sendCmd(F("AT+AZSET=\"%s\",\"%s\",\"%s\""), 1000, iothubName, deviceID, deviceKey) != TAG_OK)
        return false;

    //  AT+MQTTTOPIC="/devices/testDevice/messages/events/","/devices/testDevice/messages/devicebound/#"
    //  OK
    if (sendCmd(F("AT+MQTTTOPIC=\"%s\",\"%s\""), 1000, publishTopic, subscribeTopic) != TAG_OK)
        return false;

    return true;
}

bool WizFi360DrvExtra::connectAzure(void)
{
    //  AT+AZCON
    //  CONNECT
    //  OK
    //     or
    //  CONNECT FAIL
    //  ERROR

    return (sendCmd(F("AT+AZCON"), 180000) == TAG_OK);
}

bool WizFi360DrvExtra::mqttPublish(const char *message)
{
    //  AT+MQTTPUB="{"deviceId":"WizFi360","temperature":28.16,"humidity":46.04}"
    //  OK
    return (sendCmd(F("AT+MQTTPUB=\"%s\""), 10000, message) == TAG_OK);
}

bool WizFi360DrvExtra::mqttDisconnect(void)
{
    // AT+MQTTDIS	// Disconnects from a Broker
    // CLOSED
    return (sendCmd(F("AT+MQTTDIS")) == TAG_CLOSE);
}