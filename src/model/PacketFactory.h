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

#include <Arduino.h>
#include <mbed.h>
#include <platform/CircularBuffer.h>
#include "../../AppLog.h"

template <typename T, uint16_t length>
class PacketFactory
{
public:
    PacketFactory() : semaphore(1), buffer()
    {
    }

    ~PacketFactory()
    {
    }

    bool consumePacket(T &packet)
    {
        bool result = false;
        semaphore.acquire();
        result = buffer.pop(packet);
        semaphore.release();
        return result;
    }

    bool peekPacket(T &packet)
    {
        bool result = false;
        semaphore.acquire();
        result = buffer.peek(packet);
        semaphore.release();
        return result;
    }

    bool producePacket(T &packet)
    {
        bool result = false;
        semaphore.acquire();
        if (!buffer.full())
        {
            buffer.push(packet);
            result = true;
        }
        else
        {
            LOG_DEBUG("Ring buffer is full!");
        }
        semaphore.release();
        return result;
    }

    bool produceIsrPacket(T &packet)
    {
        bool result = false;
        semaphore.acquire();
        if (!buffer.full())
        {
            buffer.push(packet);
            result = true;
        }
        else
        {
            // LOG_DEBUG("Ring buffer is full!");
        }
        semaphore.release();
        return result;
    }

    bool isEmpty()
    {
        semaphore.acquire();
        bool result = buffer.empty();
        semaphore.release();
        return result;
    }

    bool isFull()
    {
        semaphore.acquire();
        bool result = buffer.full();
        semaphore.release();
        return result;
    }

private:
    mbed::CircularBuffer<T, length> buffer;
    rtos::Semaphore semaphore;
};
