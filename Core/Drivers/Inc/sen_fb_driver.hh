/*
 * sensor_feedback.hh
 *
 *  Created on: Feb 13, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_SEN_FB_DRIVER_HH_
#define DRIVERS_INC_SEN_FB_DRIVER_HH_

#include "hx711_driver.hh"
#include "current_amplifier_ina180.hh"

#define SEN_FB_ADC_NB_CH 4

typedef enum
{
  SEN_FB_ADC_CH_MAG	     = 0x00U,		// Magnetic position feedback
  SEN_FB_ADC_CH_POT          = 0x01U,    	// Potentiometer position feedback
  SEN_FB_ADC_CH_CUR          = 0x02U,    	// Current feedback
  SEN_FB_ADC_CH_VOL          = 0x03U    	// Voltage feedback
} SenFbAdcChType_t;

typedef struct
{
  float torque_nm;
  float angle_deg;
  float supply_current_a;
  float supply_voltage_v;
  float temperature_degc;
} SensorState_t;

class SenFbDriver
{
private:
  uint16_t 			_adc_buf[SEN_FB_ADC_NB_CH];
  ADC_HandleTypeDef    		*_hadcx;
  uint8_t 			_adcx_conv_cplt = 0;
  HX711Driver			*_load_cell;
  SensorState_t			_state;

public:
  SenFbDriver(ADC_HandleTypeDef *hadcx, HX711Driver *load_cell) : _hadcx(hadcx), _load_cell(load_cell) {}

  void update(void)
  {
    // Read load cell
    uint32_t load_cell_adc_val;
    _load_cell->read(&load_cell_adc_val);

    // Trigger new ADC measurements
    _adcx_conv_cplt = 1;
    HAL_ADC_Start_DMA(_hadcx, (uint32_t *)_adc_buf, SEN_FB_ADC_NB_CH);
  }

  void update_torque(void)
  {

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
      _state.supply_current_a = _adc_buf[SEN_FB_ADC_CH_CUR] * 3.3 / 4096 / INA180_GAIN / INA180_R_SHUNT;
    }
  }

  void update_temperature(void)
  {

  }

  ADC_TypeDef *get_adc_instance(void)
  {
    return _hadcx->Instance;
  }

  void on_adc_cplt_conv(void)
  {
    _adcx_conv_cplt = 1;
  }
};



#endif /* DRIVERS_INC_SEN_FB_DRIVER_HH_ */
