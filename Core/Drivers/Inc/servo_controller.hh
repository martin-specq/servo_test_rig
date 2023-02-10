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
#include "filter.hh"
#include "math.h"

#define SERVO_CTRL_MIN_PERIOD_S 0.2
#define SERVO_CTRL_MAX_PERIOD_S 20.0
#define SERVO_CTRL_WF_MAX_LEN 1000

typedef struct
{
  float 		values[SERVO_CTRL_WF_MAX_LEN] 			= {0};
  size_t 		len 						= SERVO_CTRL_WF_MAX_LEN;
  size_t 		head						= 0;
} Waveform_t;

typedef enum
{
  SERVO_CTRL_MODE_DISABLE		= 0x00U,	// Servo control disable
  SERVO_CTRL_MODE_WAVEFORM            	= 0x01U,    	// Waveform control mode
  SERVO_CTRL_MODE_MANUAL             	= 0x02U,    	// Manual control mode
  SERVO_CTRL_MODE_CMD              	= 0x03U,    	// Command control mode
} ServoCtrlMode_t;

class ServoController
{
private:
  TimerDriver		*_step_tim_driver;
  PWMDriver		*_pwm_driver;
  ServoMotor_t		*_servo;
  ADCDriver		*_voltage_fb_adc;
  Filter<uint16_t,16>	_voltage_fb_filter;
  ServoCtrlMode_t	_control_mode = SERVO_CTRL_MODE_DISABLE;
  Waveform_t		_waveform;

public:
  ServoController(TimerDriver *step_tim_driver,
		  PWMDriver *pwm_driver,
		  ServoMotor_t *servo,
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
    _step_tim_driver->start();
  }

  void stop()
  {
    _control_mode = SERVO_CTRL_MODE_DISABLE;
    _pwm_driver->stop();
    _step_tim_driver->stop();
  }

  void start_waveform()
  {
    _control_mode = SERVO_CTRL_MODE_WAVEFORM;
    start();
  }

  uint8_t create_waveform_sinusoidal(float angle_min_deg, float angle_max_deg, float period_s)
  {
    float omega;
    float loop_frequency_hz = _step_tim_driver->get_tim_frequency_hz();

    if(period_s >= SERVO_CTRL_MIN_PERIOD_S && period_s <= SERVO_CTRL_MAX_PERIOD_S)
    {
      _waveform.len = (size_t)(period_s * loop_frequency_hz);
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
      for(size_t i=0; i<_waveform.len; i++)
      {
	_waveform.values[i] = 0.5 * (angle_min_deg + angle_max_deg + (angle_max_deg - angle_min_deg) * sin(omega * i / loop_frequency_hz));
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
    if(_control_mode == SERVO_CTRL_MODE_WAVEFORM)
    {
      set_angle(_waveform.values[_waveform.head]);
      _waveform.head = (_waveform.head + 1) % _waveform.len;
    }
    else if(_control_mode == SERVO_CTRL_MODE_MANUAL)
    {

    }
    else if(_control_mode == SERVO_CTRL_MODE_CMD)
    {

    }
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
