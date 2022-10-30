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
#include "PinInput.h"

// GPIO for detect battery level
#define PinBatteryLed1 (10u)
#define PinBatteryLed2 (11u)
#define PinBatteryLed3 (12u)
#define PinBatteryLed4 (13u)

#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))

class Battery
{
public:
    Battery()
    {
        pinList[0] = new PinInput(PinBatteryLed1);
        pinList[1] = new PinInput(PinBatteryLed2);
        pinList[2] = new PinInput(PinBatteryLed3);
        pinList[3] = new PinInput(PinBatteryLed4);
    }

    uint16_t read(void)
    {
        static constexpr uint16_t TableBatteryLevel[] = {
            25, 50, 75, 100};

        for (int i = (sizeofarray(pinList) - 1); i >= 0; i--)
        {
            PinInput *ptr = pinList[i];
            if (ptr->read() == PinStatus::LOW)
            {
                return TableBatteryLevel[i];
            }
        }
        return 0;
    }

private:
    PinInput *pinList[4];
};
