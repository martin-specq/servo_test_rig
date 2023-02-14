/*
 * one_wire_driver.hh
 *
 *  Created on: Feb 14, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_ONE_WIRE_DRIVER_HH_
#define DRIVERS_INC_ONE_WIRE_DRIVER_HH_

#include "main.h"
#include <string.h>
#include "ds18b20_defs.h"

/** Max number of sensors allowed to be placed on the bus. */
#define ONE_WIRE_SENSORS_MAX 							12U

class OneWireDriver
{

private:
	GPIO_TypeDef *_gpiox; 													/*!< gpiox to be used for I/O functions */
	uint16_t _gpio_pin; 														/*!< gpio pin to be used for I/O functions */
	uint8_t _last_discrepancy = 0; 									/*!< search private */
	uint8_t _last_family_discrepancy = 0; 					/*!< search private */
	uint8_t _last_device_flag = 0; 									/*!< search private */

public:
	size_t _num_roms = 0; 													/*!< number of devices found */
	uint8_t _rom_no[8]; 														/*!< 8-bytes address of last search device */
	uint8_t _found_roms[ONE_WIRE_SENSORS_MAX][8]; 	/*!< list of 8-bytes address of all devices found */

public:
	OneWireDriver(GPIO_TypeDef *gpiox, uint16_t gpio_pin) :
			_gpiox(gpiox), _gpio_pin(gpio_pin)
	{
	}

private:
	// GPIO_SETPINASINPUT - Set GPIO pin selected as input (floating).
	//
	void gpio_set_pin_as_input();

	// GPIO_SETPINASOUTPUT - Set GPIO pin selected as output open-drain.
	//
	void gpio_set_pin_as_output();

public:
	/**
	 * @brief Performs a reset on the one-wire-bus using the GPIO driver and listens
	 * 		  for presence detect pulses.
	 *
	 * A reset is issued before each command and puts all sensors on the bus in a
	 * defined state.
	 *
	 * @note  The reset/detect sequence takes ~960µs.
	 * @param None.
	 * @return uint8_t If sensors were detected on the bus. If 1, there are active
	 * 		   sensors. If 0, none were found.
	 */
	uint8_t reset();

	// READ_BIT - reads a bit from the one-wire bus. Use GPIO for communication.
	// The delay required for a read is 15us.
	//
	uint8_t read_bit();

	// WRITE_BIT - writes a bit to the one-wire bus, passed in bitval. Use GPIO for communication.
	//
	void write_bit(uint8_t bit);

	// READ_BYTE - reads a byte from the one-wire bus. Uses USART for communication.
	//
	uint8_t read_byte();

	// WRITE_BYTE - writes a byte to the one-wire bus. Uses USART for communication.
	//
	void write_byte(uint8_t byte);

	// crc8 - calculates crc given the data and the length required and return the value.
	//
	uint8_t crc8(uint8_t *addr, uint8_t len);

	// RESETSEARCH - resets the search state and the recording variables
	//
	void reset_search();

	// SEARCH - performs a search on the bus using the ROM search command and algorithm detailed in the datasheet of DS18B20 sensor.
	//
	uint8_t search();

	// FIRST - performs a reset of the search recording variables and then a new search on the bus.
	//
	uint8_t first();

	// NEXT - performs a search for any more device on the bus (recording variables not re-initialized).
	//
	uint8_t next();

	// FINDDEVICES - Performs one or numerous search on the bus and record the ROM identification codes and the number of devices found.
	//
	void find_devices();

	// NUMBEROFROMS - return the number of sensors found on the bus.
	//
	uint8_t number_of_roms();

};

#endif /* DRIVERS_INC_ONE_WIRE_DRIVER_HH_ */
