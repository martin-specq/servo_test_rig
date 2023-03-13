#pragma once

#include "math.h"

#include "Util/IntervalWaiter.hh"
#include "Coms/Telemetry.hh"
#include "Coms/UsrCmdParser.hh"
#include "Config/Configuration.hh"
#include "Interfaces/DeviceInterfaces.hh"
#include "Interfaces/StreamInterface.hh"

namespace hlc
{

#define SERVO_CTRL_WF_MIN_PERIOD_S 0.2
#define SERVO_CTRL_WF_MAX_PERIOD_S 20.0

class HLC
{
private:
	const char 				*_source_id;
	TimeSourceInterface 	*_time_source;
	dfr::IntervalWaiter 	_interval_waiter;
	ServoP500Driver 		*_servo;
	SensorFeedbackDriver 	*_sensors;
	usr_cmd::UsrCmdParser 	*_usr_cmd_parser;
	telem::SerialWriter 	_telem;
	wf::Sinusoid_t 			_waveform;
	float 					_reference_deg;

public:
	HLC(const char              *source_id,
		TimeSourceInterface 	*time_source,
		ServoP500Driver 		*servo,
		SensorFeedbackDriver 	*sensors,
		usr_cmd::UsrCmdParser 	*usr_cmd_parser,
		StreamInterface 		*stream_telem) :
		_source_id(source_id),
		_interval_waiter(time_source, cfg::loop::period_us),
		_servo(servo),
		_sensors(sensors),
		_usr_cmd_parser(usr_cmd_parser),
		_telem(stream_telem)
	{
	}

	void init()
	{
		_sensors->init();
		arm();
	}

	void arm()
	{
		_servo->start_pwm();
		set_angle(0);
	}

	void set_angle(float angle_deg)
	{
		_reference_deg = angle_deg;
		_servo->set_angle(angle_deg);
	}

	void stop_waveform(void)
	{
		_waveform = {};
		_reference_deg = 0;
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

	void step(void)
	{
		// Wait for next step
	  while (!_interval_waiter.next_interval())
	  {
	  }

	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_SET);

		// Read sensors
		_sensors->update();

		// Read user commands
		SiCmd_t cmd_code = _host_pc->read();

		// Update state
		if(cmd_code == CMD_NO_CMD)
		{
		}
		else if(cmd_code == CMD_SERVO_STOP)
		{
			stop_waveform();
		}
		else if(cmd_code == CMD_SERVO_SET_ANGLE)
		{
			stop_waveform();
			float angle_deg = 0;
			_host_pc->get_target_angle(&angle_deg);
			_reference_deg = angle_deg;
		}
		else if(cmd_code == CMD_SERVO_START_SIN)
		{
			_host_pc->get_sin_params(&_waveform.angle_min_deg, &_waveform.angle_max_deg, &_waveform.period_s);
			create_waveform_sinusoidal(_waveform.angle_min_deg, _waveform.angle_max_deg, _waveform.period_s);
			_waveform.enabled = true;
			_waveform.sweep_enabled = false;
		}
		else if(cmd_code == CMD_SERVO_START_SIN_SWEEP)
		{
			_host_pc->get_sin_sweep_params(&_waveform.angle_min_deg,
															       &_waveform.angle_max_deg,
																		 &_waveform.period_min_s,
																		 &_waveform.period_max_s,
																		 &_waveform.n_periods,
																		 &_waveform.n_cycles_per_period);
			if(_waveform.period_max_s > _waveform.period_min_s && _waveform.n_periods > 0)
			{
				_waveform.period_s = _waveform.period_max_s;
				create_waveform_sinusoidal(_waveform.angle_min_deg, _waveform.angle_max_deg, _waveform.period_s);
				_waveform.enabled = true;
				_waveform.sweep_enabled = true;
			}
		}

		// Servo actions
		if(_waveform.enabled)
		{
			if(_waveform.sweep_enabled && _waveform.head == 0)
			{
				// Check increase in frequency
				if(_waveform.cycles_count == _waveform.n_cycles_per_period)
				{
					// Reset number of cycles
					_waveform.cycles_count = 0;

					// Decrease period
					_waveform.period_s -= (_waveform.period_max_s - _waveform.period_min_s) / (_waveform.n_periods - 1);

					// Check end of sweep
					if(++_waveform.periods_count == _waveform.n_periods)
					{
						stop_waveform();
					}
					else
					{
						create_waveform_sinusoidal(_waveform.angle_min_deg, _waveform.angle_max_deg, _waveform.period_s);
					}
				}
				_waveform.cycles_count++;
			}

			// Update reference if waveform still enabled
			if(_waveform.enabled)
			{
				_reference_deg = _waveform.values[_waveform.head];
				_waveform.head = (_waveform.head + 1) % _waveform.len;
			}
		}

		// Apply reference
		_servo->set_angle(_reference_deg);

		// Log to Grafana
		log();
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_10, GPIO_PIN_RESET);
	}

	void log(void)
	{
		SensorState_t state = _sensors->get_state();
		_telem.write_sequence_message();
		_telem.write_message(telem::MSG_TAG_SOURCE_ID, strlen(_source_id), _source_id);
	    _telem.write_message(telem::MSG_TAG_TIME_LOCAL, _interval_waiter.get_now_micros());
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{0, (float)state.load_cell_adc_val});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{1, (float)state.mag_feedback_adc_val});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{2, (float)state.pot_feedback_adc_val});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{3, _reference_deg});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{4, state.supply_current_a});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{5, state.supply_voltage_v});
		_telem.write_message(telem::MSG_TAG_DEBUG_VALUES, telem::debug_msg{6, state.temperature_degc[0].temp});
	}
};

} // namespace hlc

