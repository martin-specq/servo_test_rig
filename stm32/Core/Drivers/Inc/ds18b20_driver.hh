/**
 * @file    ds18b20_driver.hh
 * @brief   Driver for the DS18B20 temperature sensor over the 1-wire-protocol.
 *
 * The 1-wire-protocol is an asynchronous half-duplex connection, where the
 * master and several slave devices interact over one single data line (and GND
 * and 3V3). It is subject to strict timing requirements due to the often
 * analog circuitry on the slave side.
 * 
 * The half-duplex connection can either be driven by bit banging a GPIO pin set
 * as open drain and as input (intermittently), or by "abusing" a UART 
 * peripheral that supports single-wire half-duplex.
 * 
 * The advantage of UART is interoperability with an RTOS, because the UART
 * peripheral will be able to respect timing even in case of preemption. The
 * choice of UART or GPIO driver can be set in the initializer.
 * 
 * For more information, see: https://en.wikipedia.org/wiki/1-Wire
 * @copyright Copyright (c) 2022 Dufour Aerospace
 *
 */

#pragma once

#include "ds18b20_defs.h"
#include "one_wire_driver.hh"
#include "main.h"

typedef struct
{
	float temp;
	uint16_t sensor_id;
} TemperatureMsg_t;


class DS18B20Driver {

  private:
    OneWireDriver 		*_bus;
    TemperatureMsg_t 	_temperatures[ONE_WIRE_SENSORS_MAX];
    uint8_t _device_count = 0;

  public: DS18B20Driver(OneWireDriver *bus) : _bus(bus) {}

  private:
    /**
     * @brief Fast checksum algorithm used to retunr a 2 bytes id from the 8 bytes rom.
     *
     * @param ptr Pointer to binary data of which a checksum should be calculated
     * @param len Length of binary data located at @p rom
     * @return uint16_t The 16-bit id
     */
    uint16_t rom_to_id(uint8_t *rom, size_t len);

    // SELECTWITHPOINTER - Select a sensor on the bus by matching its ROM code.
    //
    void select_with_pointer(uint8_t *ROM);
    
    // START - selects a detected device on the bus and starts temperature conversion
    //
    uint8_t start(uint8_t *ROM);
    
    // STARTALL - initializes all devices on the line and starts temperature conversion
    //
    void start_all();

  public:
    /**
     * Reads the temperature from the one-wire bus if only one sensor is on the bus.
     * @param None
     * @returns The temperature in °C
     */
    float read_temperature_single();

    /**
     * @brief Reads the temperature from the one-write bus if multiple sensors are attached.
     * 
     * The single sensor is selected via its ROM address.
     * 
     * @param ROM An array of eight bytes containing the ROM address of the sensor to be read
     * @returns The temperature in °C
     */
    float read_temperature_multiple(uint8_t *ROM);

    // DS18B20_READALLTEMPERATURES - finds and records any device on the bus, then commands a temperature conversion,
    // and eventually reads and records the values of all temperatures.
    //
    void read_all_temperatures();

    TemperatureMsg_t get_temperature(size_t i);

    uint8_t get_device_count();
  
};
