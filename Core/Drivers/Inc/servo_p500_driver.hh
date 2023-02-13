/*
 * p500_driver.hh
 *
 *  Created on: Feb 13, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_SERVO_P500_DRIVER_HH_
#define DRIVERS_INC_SERVO_P500_DRIVER_HH_

#include "pwm_driver.hh"

#define P500_ANGLE_MIN_DEG 		-60.0
#define P500_ANGLE_MAX_DEG 		60.0
#define P500_PULSE_WIDTH_MIN_US 	900.0
#define P500_PULSE_WIDTH_MAX_US		2100.0
#define P500_PWM_FREQUENCY_HZ		50.0

class ServoP500Driver
{
private:
  PWMDriver	*_pwm_timer;

public:
  ServoP500Driver(PWMDriver *pwm_timer) : _pwm_timer(pwm_timer) {}

  void set_angle(float angle_deg)
  {
    float pulse_width_us = (angle_deg - P500_ANGLE_MIN_DEG) /
	                   (P500_ANGLE_MAX_DEG - P500_ANGLE_MIN_DEG) *
		           (P500_PULSE_WIDTH_MAX_US - P500_PULSE_WIDTH_MIN_US) +
			   P500_PULSE_WIDTH_MIN_US;

    float duty_cycle = pulse_width_us * _pwm_timer->get_tim_frequency_hz() / 1000000.0;

    _pwm_timer->set_duty_cycle(duty_cycle);
  }

  void start()
  {
    _pwm_timer->set_tim_frequency_hz(P500_PWM_FREQUENCY_HZ);
    _pwm_timer->start();
  }

  void stop()
  {
    _pwm_timer->stop();
  }
};

#endif /* DRIVERS_INC_SERVO_P500_DRIVER_HH_ */
