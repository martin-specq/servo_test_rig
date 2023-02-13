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
  TIM_HandleTypeDef 	*_waiter_us;
  GPIO_TypeDef  	*_clk_gpio;
  GPIO_TypeDef  	*_dat_gpio;
  uint16_t      	_clk_pin;
  uint16_t      	_dat_pin;
  int32_t       	_offset;
  float         	_coef;
  uint8_t       	_lock;

public:
  HX711Driver(TIM_HandleTypeDef *waiter_us,
	      GPIO_TypeDef *clk_gpio,
	      GPIO_TypeDef *dat_gpio,
	      uint16_t clk_pin,
	      uint16_t dat_pin) :
	      _waiter_us(waiter_us),
	      _clk_gpio(clk_gpio),
	      _dat_gpio(dat_gpio),
	      _clk_pin(clk_pin),
	      _dat_pin(dat_pin)
  {
    HAL_TIM_Base_Start(_waiter_us);
  }

  uint8_t read(uint32_t *data)
  {
    *data = 0U;

    // Check whether data is available
    if(HAL_GPIO_ReadPin(_dat_gpio, _dat_pin) == GPIO_PIN_SET)
    {
      return 0;
    }

    for(int8_t i=0; i<24 ; i++)
    {
      // Clock rising edge
      HAL_GPIO_WritePin(_clk_gpio, _clk_pin, GPIO_PIN_SET);
      delay_us(1);

      // Clock falling edge
      HAL_GPIO_WritePin(_clk_gpio, _clk_pin, GPIO_PIN_RESET);
      delay_us(1);

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
    delay_us(1);
    HAL_GPIO_WritePin(_clk_gpio, _clk_pin, GPIO_PIN_RESET);
    delay_us(1);

    return 1;
  }

  void delay_us(uint32_t us)
  {
    __HAL_TIM_SET_COUNTER(_waiter_us, 0);
    while(__HAL_TIM_GET_COUNTER(_waiter_us) < us);
  }
};


#endif /* DRIVERS_INC_HX711_DRIVER_HH_ */
