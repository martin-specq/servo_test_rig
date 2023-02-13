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


#define SERVO_CTRL_WF_MIN_PERIOD_S 0.2
#define SERVO_CTRL_WF_MAX_PERIOD_S 20.0
#define SERVO_CTRL_WF_MAX_LEN 1000

#define SERVO_CTRL_NB_ADC_MEAS 4

typedef struct
{
  float 		values[SERVO_CTRL_WF_MAX_LEN] 			= {0};
  size_t 		len 						= SERVO_CTRL_WF_MAX_LEN;
  size_t 		head						= 0;
} Waveform_t;

typedef struct
{
  float torque_nm;
  float angle_deg;
  float angular_rate_radps;
  float supply_current_a;
  float supply_voltage_v;
  float temperature_degc;
} ServoCtrlState_t;

typedef enum
{
  SERVO_CTRL_MODE_DISABLE		= 0x00U,	// Servo control disable
  SERVO_CTRL_MODE_WAVEFORM            	= 0x01U,    	// Waveform control mode
  SERVO_CTRL_MODE_MANUAL             	= 0x02U,    	// Manual control mode
  SERVO_CTRL_MODE_CMD              	= 0x03U,    	// Command control mode
} ServoCtrlMode_t;

typedef enum
{
  SERVO_CTRL_ADC_CH_MAG_FB		= 0x00U,	// Magnetic position feedback
  SERVO_CTRL_ADC_CH_POT_FB            	= 0x01U,    	// Potentiometer position feedback
  SERVO_CTRL_ADC_CH_CUR_FB             	= 0x02U,    	// Current feedback
  SERVO_CTRL_ADC_CH_POT_CMD           	= 0x03U,    	// Potentiometer position command
} ServoCtrlAdcChType_t;

class ServoController
{
private:
  TimerDriver					*_loop_timer;
  PWMDriver					*_pwm_timer;
  ServoMotor_t					*_servo;
  ADCDriver					*_fb_adc;
  Filter<uint16_t,16>				_voltage_fb_filter;
  ServoCtrlMode_t				_control_mode = SERVO_CTRL_MODE_DISABLE;
  Waveform_t					_waveform;
  ServoCtrlState_t				_state;

  uint16_t 					_fb_adc_buf[SERVO_CTRL_NB_ADC_MEAS];

public:
  ServoController(TimerDriver *loop_timer,
		  PWMDriver *pwm_timer,
		  ServoMotor_t *servo,
		  ADCDriver *fb_adc):
		  _loop_timer(loop_timer),
		  _pwm_timer(pwm_timer),
		  _servo(servo),
		  _fb_adc(fb_adc)
  {
  }

  void set_angle(float angle)
  {
    float pulse_width_us = (angle - _servo->_angle_min_deg) /
	                   (_servo->_angle_max_deg - _servo->_angle_min_deg) *
		           (_servo->_pulse_width_max_us - _servo->_pulse_width_min_us) +
		           _servo->_pulse_width_min_us;

    float duty_cycle = pulse_width_us * _pwm_timer->get_tim_frequency_hz() / 1000000.0;

    _pwm_timer->set_duty_cycle(duty_cycle);
  }

  void start()
  {
    _pwm_timer->set_tim_frequency_hz(_servo->_pwm_frequency);
    _pwm_timer->start();
    _loop_timer->start();
  }

  void stop()
  {
    _control_mode = SERVO_CTRL_MODE_DISABLE;
    _pwm_timer->stop();
    _loop_timer->stop();
  }

  void start_waveform()
  {
    _control_mode = SERVO_CTRL_MODE_WAVEFORM;
    start();
  }

  uint8_t create_waveform_sinusoidal(float angle_min_deg, float angle_max_deg, float period_s)
  {
    float omega;
    float loop_frequency_hz = _loop_timer->get_tim_frequency_hz();

    if(period_s >= SERVO_CTRL_WF_MIN_PERIOD_S && period_s <= SERVO_CTRL_WF_MAX_PERIOD_S)
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

    // Trigger new measurement
    _fb_adc->start_conversion((uint32_t *)_fb_adc_buf, SERVO_CTRL_NB_ADC_MEAS);
  }

  void update_state(void)
  {

  }

  TimerDriver *get_loop_timer(void)
  {
    return _loop_timer;
  }
};


#endif /* DRIVERS_INC_SERVO_CONTROLLER_HH_ */
