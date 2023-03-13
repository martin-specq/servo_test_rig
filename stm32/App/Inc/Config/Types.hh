
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace hlc
{

typedef struct
{
	// Values
	float values[SERVO_CTRL_WF_MAX_LEN] = { 0 };
	size_t len = SERVO_CTRL_WF_MAX_LEN;
	size_t head = 0;

	// Parameters
	float angle_min_deg = 0;
	float angle_max_deg = 0;
	float period_s = 0;

	// Period sweep parameters
	bool sweep_enabled = false;
	float period_max_s = 0;
	float period_min_s = 0;
	uint32_t cycles_count = 0;
	uint32_t n_cycles_per_period = 0;
	uint32_t periods_count = 0;
	uint32_t n_periods = 0;

	bool enabled = false;
} Sinusoid_t;

} // namespace hlc
