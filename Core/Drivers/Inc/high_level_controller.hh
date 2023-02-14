/*
 * servo_control.h
 *
 *  Created on: Feb 8, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_HIGH_LEVEL_CONTROLLER_HH_
#define DRIVERS_INC_HIGH_LEVEL_CONTROLLER_HH_

#include "sensor_feedback_driver.hh"
#include "servo_p500_driver.hh"
#include "math.h"

#define SERVO_CTRL_LOOP_FREQ_HZ 50

#define SERVO_CTRL_WF_MIN_PERIOD_S 0.2
#define SERVO_CTRL_WF_MAX_PERIOD_S 20.0
#define SERVO_CTRL_WF_MAX_LEN 1000

typedef struct
{
  float 		values[SERVO_CTRL_WF_MAX_LEN] 			= {0};
  size_t 		len 						= SERVO_CTRL_WF_MAX_LEN;
  size_t 		head						= 0;
} Waveform_t;

typedef enum
{
	SERVO_CTRL_MODE_DISABLE = 0x00U,	// Servo control disable
	SERVO_CTRL_MODE_WAVEFORM = 0x01U,    	// Waveform control mode
	SERVO_CTRL_MODE_MANUAL = 0x02U,    	// Manual control mode
	SERVO_CTRL_MODE_CMD = 0x03U,    	// Command control mode
} ServoCtrlMode_t;

class ServoController
{
private:
  TIM_HandleTypeDef							*_loop_timer;
  ServoP500Driver								*_servo;
  SensorFeedbackDriver					*_sensors;
  ServoCtrlMode_t								_control_mode = SERVO_CTRL_MODE_DISABLE;
  Waveform_t										_waveform;

public:
  ServoController(TIM_HandleTypeDef *loop_timer,
		  ServoP500Driver *servo,
		  SensorFeedbackDriver *sensors):
		  _loop_timer(loop_timer),
		  _servo(servo),
		  _sensors(sensors)
  {
  }

  void start()
  {
    HAL_TIM_Base_Start_IT(_loop_timer);
  }

  void stop()
  {
    _control_mode = SERVO_CTRL_MODE_DISABLE;
    HAL_TIM_Base_Stop_IT(_loop_timer);
  }

  void start_waveform()
  {
    _control_mode = SERVO_CTRL_MODE_WAVEFORM;
    start();
  }

  uint8_t create_waveform_sinusoidal(float angle_min_deg, float angle_max_deg, float period_s)
  {
    float omega;
    float loop_frequency_hz = (float)TIM_CLK_FREQ_HZ / __HAL_TIM_GET_AUTORELOAD(_loop_timer);

    if(period_s >= SERVO_CTRL_WF_MIN_PERIOD_S && period_s <= SERVO_CTRL_WF_MAX_PERIOD_S)
    {
      _waveform.len = (size_t)(period_s * loop_frequency_hz);
      omega = 2 * M_PI / period_s;
    }
    else
    {
      return 0;
    }

    if(angle_min_deg >= P500_ANGLE_MIN_DEG &&
       angle_max_deg <= P500_ANGLE_MAX_DEG &&
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

	uint8_t create_waveform_trapezoidal(float angle_min_deg, float angle_max_deg, float period_s, float plateau_time_s)
	{
		float loop_frequency_hz = (float)TIM_CLK_FREQ_HZ / __HAL_TIM_GET_AUTORELOAD(_loop_timer);

		// Check time parameters
		if(period_s < SERVO_CTRL_WF_MIN_PERIOD_S ||
			 period_s > SERVO_CTRL_WF_MAX_PERIOD_S ||
			 2 * plateau_time_s > period_s)
		{
			return 0;
		}

		// Check angle parameters
		if(angle_min_deg < P500_ANGLE_MIN_DEG ||
			 angle_max_deg > P500_ANGLE_MAX_DEG ||
			 angle_min_deg >= angle_max_deg)
		{
			return 0;
		}

		_waveform.len = (size_t)(period_s * loop_frequency_hz);

		size_t i1 = (size_t)(plateau_time_s * loop_frequency_hz);
		size_t i2 = i1 + (size_t)((period_s / 2 - plateau_time_s) * loop_frequency_hz);
		size_t i3 = i2 + i1;
		_waveform.len = i3 + i2 - i1;

		for(size_t i = 0; i < i1; i++)
		{
			_waveform.values[i] = angle_min_deg;
		}
		for(size_t i = i1; i < i2; i++)
		{
			_waveform.values[i] = angle_min_deg + (angle_max_deg - angle_min_deg) / (i2 - i1) * (i - i1);
		}
		for(size_t i = i2; i < i3; i++)
		{
			_waveform.values[i] = angle_max_deg;
		}
		for(size_t i = i3; i < _waveform.len; i++)
		{
			_waveform.values[i] = angle_max_deg - (angle_max_deg - angle_min_deg) / (_waveform.len - i3) * (i - i3);
		}

		return 1;
	}



	void step(void)
	{
		_sensors->update();
		if(_control_mode == SERVO_CTRL_MODE_WAVEFORM)
		{
			_servo->set_angle(_waveform.values[_waveform.head]);
			_waveform.head = (_waveform.head + 1) % _waveform.len;
		}
		else if(_control_mode == SERVO_CTRL_MODE_MANUAL)
		{

		}
		else if(_control_mode == SERVO_CTRL_MODE_CMD)
		{

		}

	}

	TIM_TypeDef* get_loop_timer_instance(void)
	{
		return _loop_timer->Instance;
	}
};

#endif /* DRIVERS_INC_HIGH_LEVEL_CONTROLLER_HH_ */
