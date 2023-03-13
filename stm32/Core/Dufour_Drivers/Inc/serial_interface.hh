/*
 * command_parser.hh
 *
 *  Created on: Feb 16, 2023
 *      Author: martin
 */

#ifndef DUFOUR_DRIVERS_INC_SERIAL_INTERFACE_HH_
#define DUFOUR_DRIVERS_INC_SERIAL_INTERFACE_HH_

#include <string.h>

#include "main.h"

#include "../../../App/Inc/StreamInterface.hh"

typedef enum
{
	CMD_NO_CMD										= 0x00,
	CMD_SERVO_STOP								= 0x01,
	CMD_SERVO_SET_ANGLE 					= 0x02,
	CMD_SERVO_START_SIN 					= 0x03,
	CMD_SERVO_START_TRAP					= 0x04,
	CMD_SERVO_START_SIN_SWEEP 		= 0x05,
	CMD_SERVO_START_TRAP_SWEEP		= 0x06,
	CMD_ENUM_MAX									= 0x07,
} SiCmd_t;

#define SI_CMD_HEADER 0xAB
#define SI_CMD_BUFFER_SIZE 40

class SerialInterface
{
private:
	StreamInterface 	*_stream;
	uint8_t 		_cmd_buf[SI_CMD_BUFFER_SIZE] = {0};

public:
	SerialInterface(StreamInterface *stream) : _stream(stream)
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

		while(_stream->available())
		{
			// Read next byte
			uint8_t byte = _stream->read();

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

	void get_sin_params(float *angle_min_deg, float *angle_max_deg, float *period_s)
	{
		memcpy((void *)angle_min_deg, (void *)&_cmd_buf[1], sizeof(float));
		memcpy((void *)angle_max_deg, (void *)&_cmd_buf[5], sizeof(float));
		memcpy((void *)period_s, (void *)&_cmd_buf[9], sizeof(float));
	}

	void get_sin_sweep_params(
		float *angle_min_deg,
		float *angle_max_deg,
		float *period_min_s,
		float *period_max_s,
		uint32_t *n_periods,
		uint32_t *n_cycles_per_period
	)
	{
		memcpy((void *)angle_min_deg, (void *)&_cmd_buf[1], sizeof(float));
		memcpy((void *)angle_max_deg, (void *)&_cmd_buf[5], sizeof(float));
		memcpy((void *)period_min_s, (void *)&_cmd_buf[9], sizeof(float));
		memcpy((void *)period_max_s, (void *)&_cmd_buf[13], sizeof(float));
		memcpy((void *)n_periods, (void *)&_cmd_buf[17], sizeof(uint32_t));
		memcpy((void *)n_cycles_per_period, (void *)&_cmd_buf[21], sizeof(uint32_t));
	}

	void get_trap_params(float *angle_min_deg, float *angle_max_deg, float *period_s, float *plateau_time_s)
	{
		memcpy((void *)angle_min_deg, (void *)&_cmd_buf[1], sizeof(float));
		memcpy((void *)angle_max_deg, (void *)&_cmd_buf[5], sizeof(float));
		memcpy((void *)period_s, (void *)&_cmd_buf[9], sizeof(float));
		memcpy((void *)plateau_time_s, (void *)&_cmd_buf[13], sizeof(float));
	}

	void get_target_angle(float *angle_deg)
	{
		memcpy((void *)angle_deg, (void *)&_cmd_buf[1], sizeof(float));
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
			case CMD_SERVO_START_SIN_SWEEP:
				return 5 * sizeof(float) + 1 * sizeof(uint32_t);
			case CMD_SERVO_STOP:
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


#endif /* DUFOUR_DRIVERS_INC_SERIAL_INTERFACE_HH_ */
