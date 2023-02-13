/*
 * sensor_feedback.hh
 *
 *  Created on: Feb 13, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_SEN_FB_DRIVER_HH_
#define DRIVERS_INC_SEN_FB_DRIVER_HH_

#include "hx711_driver.hh"
#include "adc_driver.hh"
#include "current_amplifier_ina180.hh"

#define SEN_FB_ADC_NB_CH 3

typedef enum
{
  SEN_FB_ADC_CH_MAG	     = 0x00U,		// Magnetic position feedback
  SEN_FB_ADC_CH_POT          = 0x01U,    	// Potentiometer position feedback
  SEN_FB_ADC_CH_CUR          = 0x02U,    	// Current feedback
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
  uint16_t 	_adc_buf[SEN_FB_ADC_NB_CH];
  ADCDriver	*_adc;
  HX711Driver	*_load_cell;
  SensorState_t	_state;

public:
  SenFbDriver(ADCDriver *adc, HX711Driver *load_cell) : _adc(adc), _load_cell(load_cell) {}

  void update(void)
  {
    _adc->start_conversion((uint32_t *)_adc_buf, SEN_FB_ADC_NB_CH);
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
    if(_adc->is_conversion_complete())
    {
      _state.supply_current_a = _adc_buf[SEN_FB_ADC_CH_CUR] * ADC_COUNTS_TO_VOLTS / INA180_GAIN / INA180_R_SHUNT;
    }
  }

  void update_temperature(void)
  {

  }
};



#endif /* DRIVERS_INC_SEN_FB_DRIVER_HH_ */
