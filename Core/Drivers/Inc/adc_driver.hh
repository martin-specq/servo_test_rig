/*
 * adc_driver.hh
 *
 *  Created on: Feb 9, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_ADC_DRIVER_HH_
#define DRIVERS_INC_ADC_DRIVER_HH_

#include "main.h"

#define ADC_DRIVER_MAX_CONV_DELAY_MS	2

class ADCDriver
{
private:
  ADC_HandleTypeDef    		*_hadcx;

public:
  ADCDriver(ADC_HandleTypeDef *hadcx) : _hadcx(hadcx){}

  uint16_t get_value(void)
  {
    HAL_ADC_Start(_hadcx);
    if(HAL_ADC_PollForConversion(_hadcx, ADC_DRIVER_MAX_CONV_DELAY_MS) == HAL_OK)
    {
      return HAL_ADC_GetValue(_hadcx);
    }
    else
    {
      return 4096;
    }
  }
};



#endif /* DRIVERS_INC_ADC_DRIVER_HH_ */
