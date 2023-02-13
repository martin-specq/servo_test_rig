/*
 * p500_driver.hh
 *
 *  Created on: Feb 13, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_SERVO_P500_DRIVER_HH_
#define DRIVERS_INC_SERVO_P500_DRIVER_HH_

#include "main.h"

#define P500_ANGLE_MIN_DEG 		-60.0
#define P500_ANGLE_MAX_DEG 		60.0
#define P500_PULSE_WIDTH_MIN_US 	900.0
#define P500_PULSE_WIDTH_MAX_US		2100.0
#define P500_PWM_FREQUENCY_HZ		50.0

class ServoP500Driver
{
private:
  TIM_HandleTypeDef	*_htimx;
  uint32_t		_timx_channel;

public:
  ServoP500Driver(TIM_HandleTypeDef *_htimx, uint32_t tim_channel) : _htimx(_htimx), _timx_channel(tim_channel) {}

  void set_angle(float angle_deg)
  {
    float pulse_width_us = (angle_deg - P500_ANGLE_MIN_DEG) /
	                   (P500_ANGLE_MAX_DEG - P500_ANGLE_MIN_DEG) *
		           (P500_PULSE_WIDTH_MAX_US - P500_PULSE_WIDTH_MIN_US) +
			   P500_PULSE_WIDTH_MIN_US;

    float duty_cycle = pulse_width_us * TIM_CLK_FREQ_HZ / 1000000.0;

    if(duty_cycle >= 0 && duty_cycle <= 1)
    {
      __HAL_TIM_SET_COMPARE(_htimx, _timx_channel, (uint32_t)(duty_cycle * __HAL_TIM_GET_AUTORELOAD(_htimx)));
    }
  }

  void start()
  {
    HAL_TIM_PWM_Start(_htimx, _timx_channel);
  }

  void stop()
  {
    HAL_TIM_PWM_Stop(_htimx, _timx_channel);
  }
};

#endif /* DRIVERS_INC_SERVO_P500_DRIVER_HH_ */
