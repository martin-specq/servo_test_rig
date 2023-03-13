/*
 * sensor_feedback.hh
 *
 *  Created on: Feb 13, 2023
 *      Author: martin
 */

#ifndef DUFOUR_DRIVERS_INC_SENSOR_FEEDBACK_DRIVER_HH_
#define DUFOUR_DRIVERS_INC_SENSOR_FEEDBACK_DRIVER_HH_

#include "../../../App/Inc/Filter.hh"
#include "../../Dufour_Drivers/Inc/current_amplifier_ina180.hh"
#include "../../Dufour_Drivers/Inc/ds18b20_driver.hh"
#include "../../Dufour_Drivers/Inc/hx711_driver.hh"

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
	int32_t load_cell_adc_val;
	uint16_t pot_feedback_adc_val;
	uint16_t mag_feedback_adc_val;
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

	// Filter for servo magnetometer feedback
	Filter<uint16_t> _mag_fb_filter;

	// Load cell
	HX711Driver *_load_cell;

	// Temperatures
	DS18B20Driver *_temp_sensors;

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
		update_load_cell();
		update_pot_feedback_adc_val();
		update_mag_feedback_adc_val();
		update_supply_voltage();
		update_supply_current();
		//update_temperatures();
		start_adc();
	}

	void update_load_cell(void)
	{
		// Read load cell
		int32_t load_cell_adc_val;
		if(_load_cell->read(&load_cell_adc_val))
		{
			_state.load_cell_adc_val = load_cell_adc_val;
		}
	}

	void update_pot_feedback_adc_val(void)
	{
		if(_adcx_conv_cplt)
		{
			_state.pot_feedback_adc_val = _adc_buf[SEN_FB_ADC_CH_POT];
		}
	}

	void update_mag_feedback_adc_val(void)
	{
		if(_adcx_conv_cplt)
		{
			_mag_fb_filter.update(_adc_buf[SEN_FB_ADC_CH_MAG]);
			_state.mag_feedback_adc_val = _mag_fb_filter.apply_mean(16);
		}
	}

	void update_supply_voltage(void)
	{
		if(_adcx_conv_cplt)
		{
			const float Rup = 6.8;
			const float Rdown = 1;
			const float calibration = 1.039;
			_state.supply_voltage_v = _adc_buf[SEN_FB_ADC_CH_VOL] * 3.3 / 4096 * (Rdown + Rup) / Rdown;
			_state.supply_voltage_v *= calibration;
		}
	}

	void update_supply_current(void)
	{
		const float calibration_gain = 1.03;
		const float calibration_offset = 0.2;
		if(_adcx_conv_cplt)
		{
			_state.supply_current_a = _adc_buf[SEN_FB_ADC_CH_CUR] * 3.3 / 4096
					/ INA180_GAIN / INA180_R_SHUNT;
			_state.supply_current_a = _state.supply_current_a * calibration_gain + calibration_offset;
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

	// Getters
	SensorState_t get_state(void)
	{
		return _state;
	}

	// ADC functions
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

#endif /* DUFOUR_DRIVERS_INC_SENSOR_FEEDBACK_DRIVER_HH_ */
