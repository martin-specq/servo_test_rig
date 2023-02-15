/*
 * sensor_feedback.hh
 *
 *  Created on: Feb 13, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_SENSOR_FEEDBACK_DRIVER_HH_
#define DRIVERS_INC_SENSOR_FEEDBACK_DRIVER_HH_

#include "hx711_driver.hh"
#include "current_amplifier_ina180.hh"
#include "ds18b20_driver.hh"

#define SEN_FB_ADC_NB_CH 4

typedef enum
{
	SEN_FB_ADC_CH_MAG = 0x00U,		// Magnetic position feedback
	SEN_FB_ADC_CH_POT = 0x01U,    // Potentiometer position feedback
	SEN_FB_ADC_CH_CUR = 0x02U,    // Current feedback
	SEN_FB_ADC_CH_VOL = 0x03U    	// Voltage feedback
} SenFbAdcChType_t;

typedef struct
{
	int32_t torque_nm;
	float angle_deg;
	float supply_current_a;
	float supply_voltage_v;
	TemperatureMsg_t temperature_degc[ONE_WIRE_SENSORS_MAX];
	size_t nb_temp_sensors = 0;
} SensorState_t;

class SensorFeedbackDriver
{
private:

	// ADC
	uint16_t _adc_buf[SEN_FB_ADC_NB_CH];
	ADC_HandleTypeDef *_hadcx;
	uint8_t _adcx_conv_cplt = 0;

	// Load cell
	HX711Driver *_load_cell;

	// Temperatures
	DS18B20Driver *_temp_sensors;

public:
	// State
	SensorState_t _state;

public:
	SensorFeedbackDriver(ADC_HandleTypeDef *hadcx, HX711Driver *load_cell,
											 DS18B20Driver *temp_sensors) :
			_hadcx(hadcx), _load_cell(load_cell), _temp_sensors(temp_sensors)
	{
	}

	void init(void)
	{
		_load_cell->tare();
	}

	void update(void)
	{
		update_torque();
		update_temperatures();
		start_adc();
	}

	void update_torque(void)
	{
		// Read load cell
		int32_t load_cell_adc_val;
		if(_load_cell->read(&load_cell_adc_val))
		{
			_state.torque_nm = load_cell_adc_val;
		}

	}

	void update_angle(void)
	{

	}

	void update_supply_voltage(void)
	{

	}

	void update_supply_current(void)
	{
		if(_adcx_conv_cplt)
		{
			_state.supply_current_a = _adc_buf[SEN_FB_ADC_CH_CUR] * 3.3 / 4096
					/ INA180_GAIN / INA180_R_SHUNT;
		}
	}

	void update_temperatures(void)
	{
		_temp_sensors->read_all_temperatures();
		_state.nb_temp_sensors = _temp_sensors->get_device_count();
		for(size_t i = 0; i < _state.nb_temp_sensors; i++)
		{
			_state.temperature_degc[i] = _temp_sensors->get_temperature(i);
		}
	}

	// Trigger new ADC measurements
	uint8_t start_adc(void)
	{
		_adcx_conv_cplt = 0;
		return (HAL_ADC_Start_DMA(_hadcx, (uint32_t*)_adc_buf, SEN_FB_ADC_NB_CH)
				== HAL_OK);
	}

	ADC_TypeDef* get_adc_instance(void)
	{
		return _hadcx->Instance;
	}

	void on_adc_cplt_conv(void)
	{
		_adcx_conv_cplt = 1;
	}
};

#endif /* DRIVERS_INC_SENSOR_FEEDBACK_DRIVER_HH_ */
