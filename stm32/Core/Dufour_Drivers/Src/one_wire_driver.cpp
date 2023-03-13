/*
 * one_wire.cpp
 *
 *  Created on: Feb 14, 2023
 *      Author: martin
 */

#ifndef DRIVERS_SRC_ONE_WIRE_CPP_
#define DRIVERS_SRC_ONE_WIRE_CPP_

#include "../../Dufour_Drivers/Inc/one_wire_driver.hh"

void OneWireDriver::gpio_set_pin_as_input()
{
	uint8_t i;
	/* Go through all pins */
	for(i = 0x00; i < 0x10; i++)
	{
		/* Pin is set */
		if(_gpio_pin & (1 << i))
		{
			/* Set 00 bits combination for input */
			_gpiox->MODER &= ~(0x03 << (2 * i));
		}
	}
}

void OneWireDriver::gpio_set_pin_as_output()
{
	uint8_t i;
	/* Go through all pins */
	for(i = 0x00; i < 0x10; i++)
	{
		/* Pin is set */
		if(_gpio_pin & (1 << i))
		{
			/* Set 01 bits combination for output */
			_gpiox->MODER = (_gpiox->MODER & ~(0x03 << (2 * i))) | (0x11 << (2 * i));
		}
	}
}

uint8_t OneWireDriver::reset()
{
	uint8_t response;

	gpio_set_pin_as_output();   // set the pin as output
	HAL_GPIO_WritePin(_gpiox, _gpio_pin, GPIO_PIN_RESET);  // pull the pin low

	delay_us(480);   // delay_us according to datasheet

	gpio_set_pin_as_input();    // set the pin as input
	delay_us(80);    // delay_us according to datasheet

	response = HAL_GPIO_ReadPin(_gpiox, _gpio_pin); // if the pin is low i.e the presence pulse is detected

	delay_us(410); // 480 us delay_us totally.

	return response; // 0 is presence pulse detected and 1 if not
}

uint8_t OneWireDriver::read_bit()
{
	uint8_t bit = 0;

	gpio_set_pin_as_output();

	HAL_GPIO_WritePin(_gpiox, _gpio_pin, GPIO_PIN_RESET); // pull DQ low to start timeslot
	delay_us(3);

	//release line
	gpio_set_pin_as_input();
	delay_us(10); // delay_us 15us from start of timeslot to read

	bit = HAL_GPIO_ReadPin(_gpiox, _gpio_pin);

	delay_us(53);

	return bit; // return value of DQ line
}

void OneWireDriver::write_bit(uint8_t bit)
{
	if(bit)
	{
		gpio_set_pin_as_output();

		HAL_GPIO_WritePin(_gpiox, _gpio_pin, GPIO_PIN_RESET); // pull DQ low to start timeslot
		delay_us(10);

		HAL_GPIO_WritePin(_gpiox, _gpio_pin, GPIO_PIN_SET); // maintain DQ high for duration of time slot

		delay_us(60); // hold value for remainder of timeslot

		gpio_set_pin_as_input();
	}
	else
	{
		gpio_set_pin_as_output();

		HAL_GPIO_WritePin(_gpiox, _gpio_pin, GPIO_PIN_RESET); // pull DQ low to start timeslot
		delay_us(65); // maintain DQ low for duration of time slot

		gpio_set_pin_as_input(); // hold value for remainder of timeslot

		delay_us(5);
	}
}

uint8_t OneWireDriver::read_byte()
{
	uint8_t i = 8, byte = 0;

	while(i--)
	{
		byte >>= 1;
		byte |= (read_bit() << 7);
	}

	return byte;
}

void OneWireDriver::write_byte(uint8_t byte)
{
	uint8_t i = 8;

	while(i--)
	{ // writes byte, one bit at a time
		write_bit(byte & 0x01); // write bit from byte starting with lsb
		byte >>= 1; // shifts byte right 'i' spaces
	}
}

uint8_t OneWireDriver::crc8(uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0, inbyte, i, mix;

	while(len--)
	{
		inbyte = *addr++;
		for(i = 8; i; i--)
		{
			mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if(mix)
			{
				crc ^= 0x8C;
			}
			inbyte >>= 1;
		}
	}

	/* Return calculated CRC */
	return crc;
}

void OneWireDriver::reset_search()
{
	/* reset the search state */
	_last_discrepancy = 0;
	_last_device_flag = 0;
	_last_family_discrepancy = 0;
}

uint8_t OneWireDriver::search()
{
	uint8_t id_bit_number;
	uint8_t last_zero, rom_byte_number, search_result;
	uint8_t id_bit, cmp_id_bit;
	uint8_t rom_byte_mask, search_direction;

	/* Initialize for search */
	id_bit_number = 1;
	last_zero = 0;
	rom_byte_number = 0;
	rom_byte_mask = 1;
	search_result = 0;

	/* Check if any devices */
	if(!_last_device_flag)
	{
		/* 1-Wire reset */
		if(reset())
		{
			/* reset the search */
			_last_discrepancy = 0;
			_last_device_flag = 0;
			_last_family_discrepancy = 0;
			return 0;
		}

		/* Issue the search command */
		write_byte (DS18B20_CMD_SEARCHROM);

		/* Loop to do the search */
		do
		{
			/* Read a bit and its complement */
			id_bit = read_bit();
			cmp_id_bit = read_bit();

			/* Check for no devices on 1-wire */
			if((id_bit == 1) && (cmp_id_bit == 1))
			{
				break;
			}
			else
			{
				/* All devices coupled have 0 or 1 */
				if(id_bit != cmp_id_bit)
				{
					/* Bit write value for search */
					search_direction = id_bit;
				}
				else
				{
					/* If this discrepancy is before the Last Discrepancy on a previous next then pick the same as last time */
					if(id_bit_number < _last_discrepancy)
					{
						search_direction = ((_rom_no[rom_byte_number] & rom_byte_mask) > 0);
					}
					else
					{
						/* If equal to last pick 1, if not then pick 0 */
						search_direction = (id_bit_number == _last_discrepancy);
					}

					/* If 0 was picked then record its position in LastZero */
					if(search_direction == 0)
					{
						last_zero = id_bit_number;

						/* Check for Last discrepancy in family */
						if(last_zero < 9)
						{
							_last_family_discrepancy = last_zero;
						}
					}
				}

				/* Set or clear the bit in the ROM byte rom_byte_number with mask rom_byte_mask */
				if(search_direction == 1)
				{
					_rom_no[rom_byte_number] |= rom_byte_mask;
				}
				else
				{
					_rom_no[rom_byte_number] &= ~rom_byte_mask;
				}

				/* Serial number search direction write bit */
				write_bit(search_direction);

				/* Increment the byte counter id_bit_number and shift the mask rom_byte_mask */
				id_bit_number++;
				rom_byte_mask <<= 1;

				/* If the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask */
				if(rom_byte_mask == 0)
				{
					rom_byte_number++;
					rom_byte_mask = 1;
				}
			}
			/* Loop until through all ROM bytes 0-7 */
		}
		while(rom_byte_number < 8);

		/* If the search was successful then */
		if(!(id_bit_number < 65))
		{
			/* search successful so set _last_discrepancy, _last_device_flag, search_result */
			_last_discrepancy = last_zero;

			/* Check for last device */
			if(_last_discrepancy == 0)
			{
				_last_device_flag = 1;
			}

			search_result = 1;
		}
	}

	/* If no device found then reset counters so next 'search' will be like a first */
	if(!search_result || !_rom_no[0])
	{
		_last_discrepancy = 0;
		_last_device_flag = 0;
		_last_family_discrepancy = 0;
		search_result = 0;
	}

	return search_result;
}

uint8_t OneWireDriver::first()
{
	/* reset search values */
	reset_search();

	/* Start with searching */
	return search();
}

uint8_t OneWireDriver::next()
{
	/* Leave the search state alone */
	return search();
}

void OneWireDriver::find_devices()
{
	uint8_t m = 0;
	_num_roms = 0;
	memset(_found_roms, 0, sizeof(_found_roms));

	if(!reset())
	{ // Begins when a presence is detected
		if(first())
		{ // Begins when at least one part is found
			do
			{
				memcpy(&_found_roms[m++], _rom_no, sizeof(_rom_no)); // Identifies ROM and records them
				_num_roms++;
			} while (next() && (_num_roms < ONE_WIRE_SENSORS_MAX));
			// Continues until no additional devices are found or the upper limit is reached
		}
	}
}

uint8_t OneWireDriver::number_of_roms()
{

	if(_num_roms == 0)
		find_devices();

	return _num_roms;
}

#endif /* DRIVERS_SRC_ONE_WIRE_CPP_ */
