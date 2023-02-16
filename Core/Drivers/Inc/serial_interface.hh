/*
 * command_parser.hh
 *
 *  Created on: Feb 16, 2023
 *      Author: martin
 */

#ifndef DRIVERS_INC_SERIAL_INTERFACE_HH_
#define DRIVERS_INC_SERIAL_INTERFACE_HH_

#include "main.h"
#include "uart_driver.hh"

typedef enum
{
	CMD_SERVO_SET_ANGLE 					= 0x00,
	CMD_SERVO_START_SIN 					= 0x01,
	CMD_SERVO_START_TRAP					= 0x02,
	CMD_SERVO_START_MANUAL_CTRL		= 0x03,
	CMD_SERVO_STOP								= 0x04,
	CMD_STREAM_START							= 0x05,
	CMD_STREAM_STOP								= 0x06,
} CmdType_t;

#define SI_CMD_HEADER 0xAB
#define SI_CMD_BUFFER_SIZE 40

class SerialInterface
{
private:
	UartDriver *_serialx;
	uint8_t _current_cmd[SI_CMD_BUFFER_SIZE] = {0};

public:
	SerialInterface(UartDriver *serialx) : _serialx(serialx)
	{
	}

	CmdType_t get_cmd_type(void)
	{
    uint8_t header_found = 0;
		while(_serialx->available())
		{
			uint8_t byte = _serialx->read();
			if(!header_found && byte == SI_CMD_HEADER)
			{
				header_found = 1;
			}
			else
			{

			}
		}
	}


};


#endif /* DRIVERS_INC_SERIAL_INTERFACE_HH_ */
