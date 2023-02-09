/*
 * servo_control.h
 *
 *  Created on: Feb 8, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_SERVO_CONTROLLER_HH_
#define DRIVERS_INC_SERVO_CONTROLLER_HH_

#include "pwm_driver.hh"
#include "timer_driver.hh"
#include "servo_motor.h"
#include "adc_driver.hh"
#include "math.h"

#define SERVO_CTRL_MIN_PERIOD_S 0.2
#define SERVO_CTRL_MAX_PERIOD_S 20.0

#define SERVO_CTRL_WF_MAX_LEN 1000

class ServoController
{
private:
  TimerDriver		*_step_tim_driver;
  PWMDriver		*_pwm_driver;
  ServoMotor		*_servo;
  ADCDriver		*_voltage_fb_adc;
  float 		 _waveform[SERVO_CTRL_WF_MAX_LEN] 	= {0};
  size_t 		 _waveform_len 				= SERVO_CTRL_WF_MAX_LEN;
  size_t 		 _waveform_idx				= 0;

public:
  ServoController(TimerDriver *step_tim_driver,
		  PWMDriver *pwm_driver,
		  ServoMotor *servo,
		  ADCDriver *voltage_fb_adc):
		  _step_tim_driver(step_tim_driver),
		  _pwm_driver(pwm_driver),
		  _servo(servo),
		  _voltage_fb_adc(voltage_fb_adc)
  {
  }

  void set_angle(float angle)
  {
    float pulse_width_us = (angle - _servo->_angle_min_deg) /
	                   (_servo->_angle_max_deg - _servo->_angle_min_deg) *
		           (_servo->_pulse_width_max_us - _servo->_pulse_width_min_us) +
		           _servo->_pulse_width_min_us;

    float duty_cycle = pulse_width_us * _pwm_driver->get_tim_frequency_hz() / 1000000.0;

    _pwm_driver->set_duty_cycle(duty_cycle);
  }

  void start()
  {
    _pwm_driver->set_tim_frequency_hz(_servo->_pwm_frequency);
    _pwm_driver->start();
    set_angle(0);
  }

  void stop()
  {
    _pwm_driver->stop();
  }

  void start_waveform()
  {
    start();
    _step_tim_driver->start();
  }

  void stop_waveform()
  {
    stop();
    _step_tim_driver->stop();
  }

  uint8_t create_waveform_sinusoidal(float angle_min_deg, float angle_max_deg, float period_s)
  {
    float omega;
    float loop_frequency_hz = _step_tim_driver->get_tim_frequency_hz();

    if(period_s >= SERVO_CTRL_MIN_PERIOD_S && period_s <= SERVO_CTRL_MAX_PERIOD_S)
    {
      _waveform_len = (size_t)(period_s * loop_frequency_hz);
      omega = 2 * M_PI / period_s;
    }
    else
    {
      return 0;
    }

    if(angle_min_deg >= _servo->_angle_min_deg &&
       angle_max_deg <= _servo->_angle_max_deg &&
       angle_min_deg < angle_max_deg)
    {
      for(size_t i=0; i<_waveform_len; i++)
      {
	_waveform[i] = 0.5 * (angle_min_deg + angle_max_deg + (angle_max_deg - angle_min_deg) * sin(omega * i / loop_frequency_hz));
      }
    }
    else
    {
      return 0;
    }
    return 1;
  }

  void step(void)
  {
    set_angle(_waveform[_waveform_idx]);
    _waveform_idx = (_waveform_idx + 1) % _waveform_len;


  }

  float get_voltage_fb(void)
  {
    return (_voltage_fb_adc->get_value()) * 3.3 / 4096;
  }

  TimerDriver *get_step_tim_driver(void)
  {
    return _step_tim_driver;
  }
};


#endif /* DRIVERS_INC_SERVO_CONTROLLER_HH_ */
