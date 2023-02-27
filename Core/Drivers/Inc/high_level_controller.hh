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
#include "serial_interface.hh"
#include "Telemetry.hh"
#include "math.h"

#define SERVO_CTRL_LOOP_FREQ_HZ 50

#define SERVO_CTRL_WF_MIN_PERIOD_S 0.2
#define SERVO_CTRL_WF_MAX_PERIOD_S 20.0
#define SERVO_CTRL_WF_MAX_LEN 1000

typedef struct
{
	float values[SERVO_CTRL_WF_MAX_LEN] =
	{ 0 };
	size_t len = SERVO_CTRL_WF_MAX_LEN;
	size_t head = 0;
} Waveform_t;

typedef enum
{
	SERVO_CTRL_MODE_DISABLE = 0x00U,			// Servo control disable
	SERVO_CTRL_MODE_WAVEFORM = 0x01U,    	// Waveform control mode
	SERVO_CTRL_MODE_MANUAL = 0x02U,    	// Manual control mode
	SERVO_CTRL_MODE_CMD = 0x03U,    	// Command control mode
} ServoCtrlMode_t;

class ServoController
{
private:
	const char *_source_id = "servo-tr";

	TIM_HandleTypeDef *_interval_waiter;
	ServoP500Driver *_servo;
	SensorFeedbackDriver *_sensors;
	SerialInterface *_host_pc;
	telem::SerialWriter _telem;
	ServoCtrlMode_t _control_mode = SERVO_CTRL_MODE_DISABLE;
	Waveform_t _waveform;

public:
	ServoController(TIM_HandleTypeDef *interval_waiter,
									ServoP500Driver *servo,
									SensorFeedbackDriver *sensors,
									SerialInterface *host_pc,
									StreamInterface *stream_telem) :
									_interval_waiter(interval_waiter),
									_servo(servo),
									_sensors(sensors),
									_host_pc(host_pc),
									_telem(stream_telem)
	{
	}

	void init()
	{
		_sensors->init();
		HAL_TIM_Base_Start(_interval_waiter);
		arm();
	}

	void arm()
	{
		_servo->start_pwm();
		set_angle(0);
	}

	void disarm()
	{
		_control_mode = SERVO_CTRL_MODE_DISABLE;
		set_angle(0);
		_servo->stop_pwm();
	}

	void set_angle(float angle_deg)
	{
		_control_mode = SERVO_CTRL_MODE_CMD;
		_servo->set_angle(angle_deg);
	}

	void start_waveform()
	{
		_control_mode = SERVO_CTRL_MODE_WAVEFORM;
	}

	uint8_t create_waveform_sinusoidal(float angle_min_deg, float angle_max_deg,
																		 float period_s)
	{
		float omega;

		if(period_s >= SERVO_CTRL_WF_MIN_PERIOD_S
				&& period_s <= SERVO_CTRL_WF_MAX_PERIOD_S)
		{
			_waveform.len = (size_t)(period_s * SERVO_CTRL_LOOP_FREQ_HZ);
			omega = 2 * M_PI / period_s;
		}
		else
		{
			return 0;
		}

		if(angle_min_deg >= P500_ANGLE_MIN_DEG
				&& angle_max_deg <= P500_ANGLE_MAX_DEG && angle_min_deg < angle_max_deg)
		{
			for(size_t i = 0; i < _waveform.len; i++)
			{
				_waveform.values[i] = 0.5
						* (angle_min_deg + angle_max_deg
								+ (angle_max_deg - angle_min_deg)
										* sin(omega * i / SERVO_CTRL_LOOP_FREQ_HZ));
			}
		}
		else
		{
			return 0;
		}
		return 1;
	}

	uint8_t create_waveform_trapezoidal(float angle_min_deg, float angle_max_deg,
																			float period_s, float plateau_time_s)
	{
		// Check time parameters
		if(period_s < SERVO_CTRL_WF_MIN_PERIOD_S
				|| period_s > SERVO_CTRL_WF_MAX_PERIOD_S
				|| 2 * plateau_time_s > period_s)
		{
			return 0;
		}

		// Check angle parameters
		if(angle_min_deg < P500_ANGLE_MIN_DEG || angle_max_deg > P500_ANGLE_MAX_DEG
				|| angle_min_deg >= angle_max_deg)
		{
			return 0;
		}

		_waveform.len = (size_t)(period_s * SERVO_CTRL_LOOP_FREQ_HZ);

		size_t i1 = (size_t)(plateau_time_s * SERVO_CTRL_LOOP_FREQ_HZ);
		size_t i2 = i1
				+ (size_t)((period_s / 2 - plateau_time_s) * SERVO_CTRL_LOOP_FREQ_HZ);
		size_t i3 = i2 + i1;
		_waveform.len = i3 + i2 - i1;

		for(size_t i = 0; i < i1; i++)
		{
			_waveform.values[i] = angle_min_deg;
		}
		for(size_t i = i1; i < i2; i++)
		{
			_waveform.values[i] = angle_min_deg
					+ (angle_max_deg - angle_min_deg) / (i2 - i1) * (i - i1);
		}
		for(size_t i = i2; i < i3; i++)
		{
			_waveform.values[i] = angle_max_deg;
		}
		for(size_t i = i3; i < _waveform.len; i++)
		{
			_waveform.values[i] = angle_max_deg
					- (angle_max_deg - angle_min_deg) / (_waveform.len - i3) * (i - i3);
		}

		return 1;
	}

	void step(void)
	{
		// Wait for next step
		while(!__HAL_TIM_GET_FLAG(_interval_waiter, TIM_FLAG_UPDATE));
		__HAL_TIM_CLEAR_FLAG(_interval_waiter, TIM_FLAG_UPDATE);

		_sensors->update();
		SiCmd_t cmd_code = _host_pc->read();
		if(cmd_code == CMD_NO_CMD)
		{
		}
		else if(cmd_code == CMD_SERVO_ARM)
		{
			arm();
		}
		else if(cmd_code == CMD_SERVO_DISARM)
		{
			disarm();
		}
		else if(cmd_code == CMD_SERVO_SET_ANGLE)
		{
			float angle_deg = 0;
			_control_mode = SERVO_CTRL_MODE_CMD;
			_host_pc->get_target_angle(&angle_deg);
			set_angle(angle_deg);
		}
		else if(cmd_code == CMD_SERVO_START_MANUAL_CTRL)
		{
			_control_mode = SERVO_CTRL_MODE_MANUAL;
		}
		else if(cmd_code == CMD_SERVO_START_SIN)
		{
			float angle_min_deg = 0;
			float angle_max_deg = 0;
			float period_s = 0;
			_host_pc->get_sin_params(&angle_min_deg, &angle_max_deg, &period_s);
			create_waveform_sinusoidal(angle_min_deg, angle_max_deg, period_s);
			_control_mode = SERVO_CTRL_MODE_WAVEFORM;
		}
		else if(cmd_code == CMD_SERVO_START_TRAP)
		{
			float angle_min_deg = 0;
			float angle_max_deg = 0;
			float period_s = 0;
			float plateau_time_s = 0;
			_host_pc->get_trap_params(&angle_min_deg, &angle_max_deg, &period_s,
																&plateau_time_s);
			create_waveform_trapezoidal(angle_min_deg, angle_max_deg, period_s,
																	plateau_time_s);
			_control_mode = SERVO_CTRL_MODE_WAVEFORM;
		}

		// Servo actions
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
		log();
	}

	void log(void)
	{

		SensorState_t state = _sensors->get_state();
		_telem.write_sequence_message();
		_telem.write_message(telem::MSG_TAG_SOURCE_ID, strlen(_source_id), _source_id);
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{0, (float)state.load_cell_adc_val});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{1, (float)state.mag_feedback_adc_val});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{2, (float)state.pot_feedback_adc_val});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{3, state.supply_current_a});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{4, state.supply_voltage_v});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{5, state.temperature_degc[0].temp});

	}
};



#endif /* DRIVERS_INC_HIGH_LEVEL_CONTROLLER_HH_ */
