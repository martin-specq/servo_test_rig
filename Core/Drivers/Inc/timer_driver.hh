/*
 * timer_driver.hh
 *
 *  Created on: Feb 9, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_TIMER_DRIVER_HH_
#define DRIVERS_INC_TIMER_DRIVER_HH_

#include "main.h"

class TimerDriver
{
protected:
  TIM_HandleTypeDef    *_htimx;

public:
  TimerDriver(TIM_HandleTypeDef *htimx): _htimx(htimx)
  {
  }

  void start(void)
  {
    HAL_TIM_Base_Start_IT(_htimx);
  }

  void stop(void)
  {
    HAL_TIM_Base_Stop_IT(_htimx);
  }

  uint8_t set_tim_frequency_hz(float freq_hz)
  {
    /**
     * TODO:
     * Consider 16 bits timers
     * Consider other clock sources
     */
    if((uint32_t)(HAL_RCC_GetPCLK1Freq() / freq_hz) < MAX_UINT32_T && freq_hz <= HAL_RCC_GetPCLK1Freq())
    {
      __HAL_TIM_SET_CLOCKDIVISION(_htimx, TIM_CLOCKDIVISION_DIV1);
      __HAL_TIM_SET_PRESCALER(_htimx, 0);
      __HAL_TIM_SET_AUTORELOAD(_htimx, (uint32_t)(HAL_RCC_GetPCLK1Freq() / freq_hz - 1));
    }
    else
    {
      return 0;
    }
    return 1;
  }

  float get_tim_frequency_hz(void)
  {
    /**
     * TODO:
     * Consider 16 bits timers
     * Consider other clock sources
     */
    return HAL_RCC_GetPCLK1Freq() / (__HAL_TIM_GET_AUTORELOAD(_htimx) + 1);
  }

  TIM_TypeDef *get_instance(void)
  {
    return _htimx->Instance;
  }
};

#endif /* DRIVERS_INC_TIMER_DRIVER_HH_ */
