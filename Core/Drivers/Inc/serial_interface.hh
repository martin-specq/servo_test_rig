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
	CMD_NO_CMD										= 0x00,
	CMD_SERVO_ARM									= 0x01,
	CMD_SERVO_DISARM							= 0x02,
	CMD_SERVO_SET_ANGLE 					= 0x03,
	CMD_SERVO_START_SIN 					= 0x04,
	CMD_SERVO_START_TRAP					= 0x05,
	CMD_SERVO_START_MANUAL_CTRL		= 0x06,
	CMD_STREAM_START							= 0x07,
	CMD_STREAM_STOP								= 0x08,
	CMD_ENUM_MAX									= 0x09,
} SiCmd_t;

#define SI_CMD_HEADER 0xAB
#define SI_CMD_BUFFER_SIZE 40

class SerialInterface
{
private:
	UartDriver 	*_serialx;
	uint8_t 		_cmd_buf[SI_CMD_BUFFER_SIZE] = {0};

public:
	SerialInterface(UartDriver *serialx) : _serialx(serialx)
	{
	}

	SiCmd_t read(void)
	{
    // Was message header detected
		uint8_t header_found = 0;

    // Command buffer index
    size_t i = 0;

    // Message payload
    size_t payload;

		while(_serialx->available())
		{
			// Read next byte
			uint8_t byte = _serialx->read();

			// Look for start header
			if(!header_found)
			{
				if(byte == SI_CMD_HEADER)
				{
					header_found = 1;
				}
			}
			else
			{
				// Process command byte
				if(i == 0)
				{
					if(cmd_valid(byte))
					{
						payload = get_payload_from_cmd(byte);
						_cmd_buf[i++] = byte;
					}
					else
					{
						return CMD_NO_CMD;
					}
				}
				else
				{
					if(i == payload + 1)
					{
						if(byte == calculate_checksum(_cmd_buf, payload + 1))
						{
							return (SiCmd_t)_cmd_buf[0];
						}
						else
						{
							return CMD_NO_CMD;
						}
					}
					else
					{
						_cmd_buf[i++] = byte;
					}
				}
			}
		}
		return CMD_NO_CMD;
	}

private:
	uint8_t cmd_valid(uint8_t byte)
	{
		if(byte < CMD_ENUM_MAX)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}

private:
	size_t get_payload_from_cmd(uint8_t cmd)
	{
		switch(cmd)
		{
			case CMD_SERVO_SET_ANGLE:
				return 1 * sizeof(float);
			case CMD_SERVO_START_SIN:
				return 3 * sizeof(float);
			case CMD_SERVO_START_TRAP:
				return 4 * sizeof(float);
			case CMD_SERVO_START_MANUAL_CTRL:
				return 0;
			case CMD_SERVO_ARM:
				return 0;
			case CMD_SERVO_DISARM:
				return 0;
			case CMD_STREAM_START:
				return 0;
			case CMD_STREAM_STOP:
				return 0;
			default:
				return 0;
		}
	}

private:
	uint8_t calculate_checksum(uint8_t *msg, size_t msg_len)
	{
		uint32_t sum = 0;
		for(size_t i = 0; i < msg_len; i++)
		{
			sum += msg[i];
		}
		return (uint8_t)(sum & 0xFF);
	}
};


#endif /* DRIVERS_INC_SERIAL_INTERFACE_HH_ */
