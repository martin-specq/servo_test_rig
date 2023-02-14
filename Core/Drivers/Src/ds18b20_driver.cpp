/**
 * @file    ds18b20_driver.cpp
 * @brief   Driver for the DS18B20 temperature sensor over the 1-wire-protocol.
 *
 */

#include "ds18b20_driver.hh"
#include <string.h>
#include <math.h>

//static uint8_t startup_counter = 0;

uint16_t DS18B20Driver::rom_to_id(uint8_t *rom, size_t len)
{
	uint8_t crc[2] =
	{ 0x00, 0x00 };
	uint16_t id = 0;

	crc[0] = _bus->crc8(rom, 4);
	crc[1] = _bus->crc8(rom + 4, 4);

	id = (uint16_t) (crc[0] | crc[1] << 8);

	return id;
}

void DS18B20Driver::select_with_pointer(uint8_t *ROM)
{
	uint8_t i;

	_bus->write_byte(DS18B20_CMD_MATCHROM);

	for(i = 0; i < 8; i++)
	{
		_bus->write_byte(*(ROM + i));
	}
}

uint8_t DS18B20Driver::start(uint8_t *ROM)
{
	/* reset line */
	_bus->reset();
	/* Select ROM number */
	select_with_pointer(ROM);
	/* Start temperature conversion */
	_bus->write_byte(DS18B20_CMD_CONVERTTEMP);
	/* Wait for conversion 750us at 12bits resolution */
	delay_us(750);

	return 1;
}

void DS18B20Driver::start_all()
{
	/* reset pulse */
	_bus->reset();
	/* Skip rom */
	_bus->write_byte(DS18B20_CMD_SKIPROM);
	/* Start conversion on all connected devices */
	_bus->write_byte(DS18B20_CMD_CONVERTTEMP);
	/* Wait for conversion 750us at 12bits resolution */
	delay_us(750);
}

float DS18B20Driver::read_temperature_single()
{
	uint8_t data[9];
	uint8_t temp_lsb, temp_msb;
	uint8_t k;
	uint16_t temp_c = -127;
	uint8_t resolution;
	int8_t digit, minus = 0;
	float decimal = -127.f;

	_bus->reset();

	_bus->write_byte(DS18B20_CMD_SKIPROM); // Skip ROM
	_bus->write_byte(DS18B20_CMD_RSCRATCHPAD); // Read Scratch Pad

	for(k = 0; k < 9; k++)
	{
		data[k] = _bus->read_byte();
	}

	temp_msb = data[1]; // Sign byte = 5 sign bits + 3 ms bits for temp (2^6, 2^5, 2^4)
	temp_lsb = data[0]; // Temp data for 2^3 jusqu´à 2^-4 pour avoir une resolution à 12bits

	temp_c = temp_lsb | (temp_msb << 8);

	_bus->reset();

	/* Check if temperature is negative */
	if(temp_c & 0x8000)
	{
		/* Two's complement, temperature is negative */
		temp_c = ~temp_c + 1;
		minus = 1;
	}

	/* Get sensor resolution */
	resolution = ((data[4] & 0x60) >> 5) + 9;

	/* Store temperature integer digits and decimal digits */
	digit = temp_c >> 4;
	digit |= ((temp_c >> 8) & 0x7) << 4;

	/* Store decimal digits */
	switch(resolution)
	{
		case 9:
		{
			decimal = (temp_c >> 3) & 0x01;
			decimal *= (float) 0.5;
		}
			break;
		case 10:
		{
			decimal = (temp_c >> 2) & 0x03;
			decimal *= (float) 0.25;
		}
			break;
		case 11:
		{
			decimal = (temp_c >> 1) & 0x07;
			decimal *= (float) 0.125;
		}
			break;
		case 12:
		{
			decimal = temp_c & 0x0F;
			decimal *= (float) 0.0625;
		}
			break;
		default:
		{
			decimal = 0xFF;
			digit = 0;
		}
	}

	/* Check for negative part */
	decimal = digit + decimal;
	if(minus)
	{
		decimal = 0 - decimal;
	}

	return decimal;
}

/**
 float DS18B20Driver::Read_Temperature_Multiple(uint8_t *ROM)
 {
 uint8_t data[9];
 uint8_t temp_lsb, temp_msb;
 uint8_t k;
 uint16_t temp_c = -127;
 uint8_t resolution;
 int8_t digit, minus = 0;
 float decimal = -127.f;
 uint8_t crc;

 _bus->reset();

 select_with_pointer(ROM); // Select with ROM
 _bus->write_byte(DS18B20_CMD_RSCRATCHPAD); // Read Scratch Pad

 for(k = 0; k < 9; k++)
 {
 data[k] = _bus->read_byte();
 }

 // Calculate CRC
 crc = _bus->crc8(data, 8);

 // Check if CRC is ok
 if(crc != data[8])
 {
 // CRC invalid
 return -127.f;
 }

 temp_msb = data[1]; // Sign byte = 5 sign bits + 3 ms bits for temp (2^6, 2^5, 2^4)
 temp_lsb = data[0]; // Temp data for 2^3 jusqu´à 2^-4 pour avoir une resolution à 12bits

 temp_c = temp_lsb | (temp_msb << 8);

 _bus->reset();

 // Check if temperature is negative
 if(temp_c & 0x8000)
 {
 // Two's complement, temperature is negative
 temp_c = ~temp_c + 1;
 minus = 1;
 }

 // Get sensor resolution
 resolution = ((data[4] & 0x60) >> 5) + 9;

 // Store temperature integer digits and decimal digits
 digit = temp_c >> 4;
 digit |= ((temp_c >> 8) & 0x7) << 4;

 // Store decimal digits
 switch(resolution)
 {
 case 9:
 {
 decimal = (temp_c >> 3) & 0x01;
 decimal *= (float) 0.5;
 }
 break;
 case 10:
 {
 decimal = (temp_c >> 2) & 0x03;
 decimal *= (float) 0.25;
 }
 break;
 case 11:
 {
 decimal = (temp_c >> 1) & 0x07;
 decimal *= (float) 0.125;
 }
 break;
 case 12:
 {
 decimal = temp_c & 0x0F;
 decimal *= (float) 0.0625;
 }
 break;
 default:
 {
 decimal = 0xFF;
 digit = 0;
 }
 }

 // Check for negative part
 decimal = digit + decimal;
 if(minus)
 {
 decimal = 0 - decimal;
 }

 return decimal;
 }

 void DS18B20Driver::ReadAllTemperatures()
 {
 uint8_t i = 0;
 device_count = 0;

 if(_bus->numROMs == 0)
 {
 _bus->FindDevices();
 }

 device_count = _bus->numROMs;

 if(device_count > 1)
 {
 for(i = 0; i < _bus->numROMs; i++)
 {
 uint8_t *rom = _bus->found_roms[i];

 Start(rom);

 uint16_t id = rom_to_id(rom, sizeof(uint64_t));
 int16_t temp = (int16_t) (Read_Temperature_Multiple(rom) * 16);

 if(startup_counter > 10)
 {
 temp = _filter[i].apply(temp);
 }

 temperatures[i] = temperature;
 }
 }
 else if(device_count == 1)
 {
 uint8_t *rom = _bus->found_roms[0];

 StartAll();

 uint16_t id = rom_to_id(rom, sizeof(uint64_t));
 int16_t temp = (int16_t) (Read_Temperature_Single() * 16);

 temperature_msg temperature =
 { id, temp };
 temperatures[0] = temperature;
 }
 else
 {
 }
 if(startup_counter <= 10)
 startup_counter++;
 }
 */
