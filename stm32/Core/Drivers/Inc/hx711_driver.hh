/*
 * hx711_driver.hh
 *
 *  Created on: Feb 13, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_HX711_DRIVER_HH_
#define DRIVERS_INC_HX711_DRIVER_HH_

#include "main.h"

class HX711Driver
{
private:
	GPIO_TypeDef *_clk_gpio;
	uint16_t _clk_pin;
	GPIO_TypeDef *_dat_gpio;
	uint16_t _dat_pin;
	int32_t _offset = 0;

public:
	HX711Driver(GPIO_TypeDef *clk_gpio,
							uint16_t clk_pin,
							GPIO_TypeDef *dat_gpio,
							uint16_t dat_pin) :
							_clk_gpio(clk_gpio),
							_clk_pin(clk_pin),
							_dat_gpio(dat_gpio),
							_dat_pin(dat_pin)
	{
	}

	uint8_t read(int32_t *data)
	{
		*data = 0x0;

		// Check whether data is available
		if(HAL_GPIO_ReadPin(_dat_gpio, _dat_pin) == GPIO_PIN_SET)
		{
			return 0;
		}

		for(int8_t i = 0; i < 24; i++)
		{
			// Clock rising edge
			HAL_GPIO_WritePin(_clk_gpio, _clk_pin, GPIO_PIN_SET);
			delay_us(10);

			// Clock falling edge
			HAL_GPIO_WritePin(_clk_gpio, _clk_pin, GPIO_PIN_RESET);
			delay_us(10);

			// Shift data
			*data = *data << 1;

			// Read and store received bit
			if(HAL_GPIO_ReadPin(_dat_gpio, _dat_pin) == GPIO_PIN_SET)
			{
				*data |= 0b1;
			}
		}

		// 25th clock pulse (input: A, gain: 128)
		HAL_GPIO_WritePin(_clk_gpio, _clk_pin, GPIO_PIN_SET);
		delay_us(10);
		HAL_GPIO_WritePin(_clk_gpio, _clk_pin, GPIO_PIN_RESET);
		delay_us(10);

		// Convert 24 bits signed data into 32 bits signed data
		if(*data & 0x800000)
		{
			*data |= 0xFF000000;
		}

		*data -= _offset;

		return 1;
	}

	void reset(void)
	{
		HAL_GPIO_WritePin(GPIOB, _clk_pin, GPIO_PIN_SET);
		delay_us(100);
		HAL_GPIO_WritePin(_clk_gpio, _clk_pin, GPIO_PIN_RESET);
	}

	void tare(void)
	{
		const uint8_t n_readings_max = 10;
		const uint16_t n_fails_max = 30;
		uint8_t n_readings = 0;
		uint16_t n_fails = 0;
		int32_t sum = 0;

		while(n_readings < n_readings_max)
		{
			int32_t data;
			if(read(&data))
			{
				n_readings++;
				sum += data;
			}
			else if(n_fails++ == n_fails_max)
			{
				return;
			}
			HAL_Delay(100);
		}
		_offset = sum / n_readings;
	}
};

#endif /* DRIVERS_INC_HX711_DRIVER_HH_ */
