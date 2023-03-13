#pragma once

#include "Interfaces/DeviceInterfaces.hh"
#include "main.h"

class ADCDriver : public ADCInterface
{
private:
	ADC_HandleTypeDef 	*_hadcx;
	bool 				_adc_conv_cplt = false;

public:
	ADCDriver(ADC_HandleTypeDef *hadcx) : _hadcx(hadcx) {}

	bool available(void) const
	{
		return _adc_conv_cplt;
	}

	void start_conversion(uint8_t buf, size_t len)
	{
		_adc_conv_cplt = false;
		HAL_ADC_Start_DMA(_hadcx, buf, len);
	}

	ADC_TypeDef* get_adc_instance(void)
	{
		return _hadcx->Instance;
	}

	void on_adc_cplt_conv(void)
	{
		_adc_conv_cplt = true;
	}
};
