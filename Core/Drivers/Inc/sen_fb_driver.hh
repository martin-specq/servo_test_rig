/*
 * sensor_feedback.hh
 *
 *  Created on: Feb 13, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_SEN_FB_DRIVER_HH_
#define DRIVERS_INC_SEN_FB_DRIVER_HH_

#include "adc_driver.hh"

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
public:
  uint16_t 	_adc_buf[SEN_FB_ADC_NB_CH];
private:
  ADCDriver	*_adc;
  SensorState_t	_state;

public:
  SenFbDriver(ADCDriver *adc) : _adc(adc) {}

  void update(void)
  {
    _adc->start_conversion((uint32_t *)_adc_buf, SEN_FB_ADC_NB_CH);
  }
};



#endif /* DRIVERS_INC_SEN_FB_DRIVER_HH_ */
