#pragma once

#include <cstdint>
#include <math.h>

/**
 * @brief Groups all configuration parameters for the controller and mixer.
 */
namespace cfg
{

// Telemetry source identifier, typically the aircraft identifier.
const char source_id[] = "test-ser-x23";

// Control loop timing configuration.
namespace loop
{
const unsigned long period_us = 20000;
const float         dt_secs   = period_us / 1000000.f;
const float         freq_hz   = 1000000.0f / period_us;
} // namespace loop

} // namespace cfg
