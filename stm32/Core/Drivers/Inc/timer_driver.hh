#pragma once

#include "DeviceInterfaces.hh"
#include "main.h"

class TimerDriver : public TimeSourceInterface
{
  public:
    uint32_t now_micros() const override
    {
      uint32_t       m   = HAL_GetTick();
      const uint32_t tms = SysTick->LOAD + 1;
      __IO uint32_t  u   = tms - SysTick->VAL;

      return (m * 1000 + (u * 1000) / tms);
    }
};
