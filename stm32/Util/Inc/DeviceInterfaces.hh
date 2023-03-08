#pragma once

#include <stdint.h>

class TimeSourceInterface
{
  public:
    virtual uint32_t now_micros() const = 0;
};
