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
#include <mbed.h>
#include "AppLog.h"
#include "AppEvent.h"
#include "./src/ThreadWizFi360.h"
#include "./src/hardware/Battery.h"

#define PIN_SWATTER (22u)   // control load switch EN pin, HIGH => on
#define PIN_LED_GREEN (25u) // LED on mean swatter is turn on

#define BATTERY_REPORT_INTERVAL 3600 // report battery level every 60 minutes
// #define BATTERY_REPORT_INTERVAL 60 // report battery level every 60 seconds (for testing only)

#define SWATTER_ON_TIME 10  // on for 10 seconds
#define SWATTER_OFF_TIME 40 // off for 40 seconds

#define SolarPanelStableTime 100 // delay time for solar panel output becomes stable

#define THREAD_WIZFI360_QUEUE_SIZE (256 * EVENTS_EVENT_SIZE) // message queue size for WizFi360 task
static ThreadWizFi360 threadWizFi360(THREAD_WIZFI360_QUEUE_SIZE);

static Battery battery;
static uint16_t batteryLevel = 0;

static inline void SwatterOn(void)
{
    LOG_INFO("Turn swatter on");
    digitalWrite(PIN_SWATTER, HIGH);
}
static inline void SwatterOff(void)
{
    LOG_INFO("Turn swatter off");
    digitalWrite(PIN_SWATTER, LOW);
}

static inline void ledOn(void)
{
    digitalWrite(PIN_LED_GREEN, HIGH);
}
static inline void ledOff(void)
{
    digitalWrite(PIN_LED_GREEN, LOW);
}

void setup(void)
{
    // delay for solar panel output becomes stable
    delay(SolarPanelStableTime);

    Serial1.begin(115200);
    LOG_ATTACH_SERIAL(Serial1);
    // LOG_SET_LEVEL(arx::debug::LogLevel::LVL_WARN);
    LOG_SET_DELIMITER("");
    PRINTLN("==============================================================");
    PRINTLN("initialized Serial1 (Pico UART0)");

    pinMode(PIN_SWATTER, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    SwatterOff();
    ledOff();

    batteryLevel = battery.read();
    LOG_DEBUG("batteryLevel=", batteryLevel);

    threadWizFi360.start();
    threadWizFi360.postEvent(EventBatteryStatusUpdate, batteryLevel); // report battery status after bootup
}

void loop(void)
{
    // turn on swatter for 10s, then off for 40s
    static unsigned int swatterCounter = 0;
    if (swatterCounter++ == 0)
    {
        SwatterOn();
        ledOn();
    }
    else if (swatterCounter == SWATTER_ON_TIME)
    {
        SwatterOff();
        ledOff();
    }
    else if (swatterCounter >= (SWATTER_ON_TIME + SWATTER_OFF_TIME))
    {
        swatterCounter = 0;
    }

    // signal ThreadWizFi360 to report battery level hourly
    static unsigned int batteryCounter = 0;
    uint16_t level = battery.read();
    // LOG_DEBUG("level=", level);
    if (batteryLevel != level)
    {
        batteryLevel = level;
        threadWizFi360.postEvent(EventBatteryStatusUpdate, level);
    }
    else if (batteryCounter++ >= BATTERY_REPORT_INTERVAL)
    {
        threadWizFi360.postEvent(EventBatteryStatusUpdate, level);
        batteryCounter = 0;
    }

    delay(1000);
}