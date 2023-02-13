/*
 * adc_driver.hh
 *
 *  Created on: Feb 9, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_ADC_DRIVER_HH_
#define DRIVERS_INC_ADC_DRIVER_HH_

#include "main.h"

class ADCDriver
{
private:
  ADC_HandleTypeDef    		*_hadcx;
  uint8_t 			_conversion_complete = 0;

public:
  ADCDriver(ADC_HandleTypeDef *hadcx) : _hadcx(hadcx){}

  void start_conversion(uint32_t *buf, size_t len)
  {
    _conversion_complete = 0;
    HAL_ADC_Start_DMA(_hadcx, buf, len);
  }

  void on_complete_conversion(void)
  {
    _conversion_complete = 1;
  }

  uint8_t is_conversion_complete(void)
  {
    return _conversion_complete;
  }

  ADC_TypeDef *get_instance(void)
  {
    return _hadcx->Instance;
  }
};



#endif /* DRIVERS_INC_ADC_DRIVER_HH_ */
