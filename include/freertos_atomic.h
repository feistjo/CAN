#pragma once

#include "FreeRTOS.h"
#include "task.h"  // must define taskENTER_CRITICAL() and taskEXIT_CRITICAL()

template <typename t>
class Atomic
{
public:
    Atomic<t>() {}

    operator t() const
    {
        taskENTER_CRITICAL();
        t val = val_;
        taskEXIT_CRITICAL();
        return val;
    }

    Atomic<t> &operator=(const t &val)
    {
        taskENTER_CRITICAL();
        val_ = val;
        taskEXIT_CRITICAL();
        return *this;
    }

private:
    t val_;
};
