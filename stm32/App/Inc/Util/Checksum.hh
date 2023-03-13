#pragma once

#include "Config/Types.hh"

uint8_t calculate_8bits_checksum(uint8_t *msg, size_t msg_len)
{
	uint32_t sum = 0;
	for(size_t i = 0; i < msg_len; i++)
	{
		sum += msg[i];
	}
	return (uint8_t)(sum & 0xFF);
}
