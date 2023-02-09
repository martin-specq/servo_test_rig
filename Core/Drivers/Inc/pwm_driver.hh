/*
 * pwm_driver.hh
 *
 *  Created on: Feb 8, 2023
 *      Author: martin
 */

/**
 * ONLY WORKS FOR TIM2 AND TIM5 SO FAR !!!
 */

#ifndef DRIVERS_INC_PWM_DRIVER_HH_
#define DRIVERS_INC_PWM_DRIVER_HH_

#include "main.h"
#include "timer_driver.hh"

class PWMDriver : public TimerDriver
{
private:
  uint32_t 		_timx_channel;

public:
  PWMDriver(TIM_HandleTypeDef *htimx,
	    uint32_t timx_channel):
	    TimerDriver(htimx),
	    _timx_channel(timx_channel)

  {
  }

  void start(void)
  {
    HAL_TIM_PWM_Start(_htimx, _timx_channel);
  }

  void stop(void)
  {
    HAL_TIM_PWM_Stop(_htimx, _timx_channel);
  }

  uint8_t set_duty_cycle(float duty_cycle)
  {
    if(duty_cycle >= 0 && duty_cycle <= 1)
    {
      __HAL_TIM_SET_COMPARE(_htimx, _timx_channel, (uint32_t)(duty_cycle * __HAL_TIM_GET_AUTORELOAD(_htimx)));
    }
    else
    {
      return 0;
    }
    return 1;
  }
};

#endif /* DRIVERS_INC_PWM_DRIVER_HH_ */
