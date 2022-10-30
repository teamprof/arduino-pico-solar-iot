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
#include "./MessageType.h"

#define DefaultQueueSize (128 * EVENTS_EVENT_SIZE)

#define __EVENT_MAP(class, event)      \
    {                                  \
        event, &class ::handler##event \
    }
#define __EVENT_FUNC_DEFINITION(class, event, msg) void class ::handler##event(const Message &msg)
#define __EVENT_FUNC_DECLARATION(event) void handler##event(const Message &msg);

class MessageQueue
{
public:
    MessageQueue(uint16_t queueSize = DefaultQueueSize) : queue(queueSize){};

    void postEvent(MessageQueue *msgQueue, const Message &message)
    {
        if (msgQueue != nullptr)
        {
            auto ev = msgQueue->queue.event(msgQueue, &MessageQueue::onMessage);
            ev.post(message);
        }
    }
    void postEvent(MessageQueue *msgQueue, int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L)
    {
        Message msg;
        msg.event = event;
        msg.iParam = iParam;
        msg.uParam = uParam;
        msg.lParam = lParam;
        postEvent(msgQueue, msg);
    }

    inline void postEvent(int16_t event, int16_t iParam = 0, uint16_t uParam = 0, uint32_t lParam = 0L)
    {
        postEvent(this, event, iParam, uParam, lParam);
    }
    inline void postEvent(const Message &message)
    {
        postEvent(this, message);
    }

    virtual void onMessage(const Message msg) = 0;

    events::EventQueue queue;

protected:
};
