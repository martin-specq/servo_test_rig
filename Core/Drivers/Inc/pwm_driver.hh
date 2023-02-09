/*
 * pwm_driver.hh
 *
 *  Created on: Feb 8, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_PWM_DRIVER_HH_
#define DRIVERS_INC_PWM_DRIVER_HH_

#include "main.h"

class PWMDriver
{
private:
  TIM_HandleTypeDef    *_htimx;
  uint32_t 		_timx_channel;

public:
  PWMDriver(TIM_HandleTypeDef *htimx, uint32_t timx_channel): _htimx(htimx), _timx_channel(timx_channel) {}

  void start(void)
  {
    HAL_TIM_PWM_Start(_htimx, _timx_channel);
  }

  void stop(void)
  {
    HAL_TIM_PWM_Stop(_htimx, _timx_channel);
  }

  void set_duty_cycle(float duty_cycle)
  {
    if(duty_cycle >= 0 && duty_cycle <= 1)
    {
      __HAL_TIM_SET_COMPARE(_htimx, _timx_channel, (uint32_t)(duty_cycle * __HAL_TIM_GET_AUTORELOAD(_htimx)));
    }
  }

  void set_pwm_frequency_hz(float freq_hz)
  {
    if(freq_hz > 0 && freq_hz <= TIM_CLK_FREQ_HZ)
    {
      __HAL_TIM_SET_CLOCKDIVISION(_htimx, TIM_CLOCKDIVISION_DIV1);
      __HAL_TIM_SET_PRESCALER(_htimx, 0);
      __HAL_TIM_SET_AUTORELOAD(_htimx, (uint32_t)(TIM_CLK_FREQ_HZ / freq_hz - 1));
    }
  }

  float get_pwm_frequency_hz(void)
  {
    return TIM_CLK_FREQ_HZ / (__HAL_TIM_GET_AUTORELOAD(_htimx) + 1);
  }
};



#endif /* DRIVERS_INC_PWM_DRIVER_HH_ */
