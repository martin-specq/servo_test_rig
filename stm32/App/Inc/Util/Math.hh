#pragma once

#include <cmath>

#undef RAD_TO_DEG
#undef DEG_TO_RAD

const double RAD_TO_DEG = 180.0 / M_PI;
const double DEG_TO_RAD = M_PI / 180.0;

namespace math
{

template<class T> T min(const T a, const T b)
{
  return a < b ? a : b;
}

template<class T> T max(const T a, const T b)
{
  return a > b ? a : b;
}

template<class T> T constrain(const T value, const T min, const T max)
{
  if (value < min)
    return min;
  if (value > max)
    return max;
  return value;
}

template<class T> T remap(const T value, const T in_min, const T in_max, const T out_min, const T out_max)
{
  if (value <= in_min)
    return out_min;
  if (value >= in_max)
    return out_max;
  if (in_min == in_max)
    return out_min;
  return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

namespace
{
[[maybe_unused]] auto remapf = remap<float>;
[[maybe_unused]] auto remapd = remap<double>;
} // namespace

template<class T> T deadzone(const T value, const T range)
{
  if (value > range)
    return remap<T>(value, range, 1, 0, 1);
  if (value < -range)
    return remap<T>(value, -1, -range, -1, 0);
  return 0;
}

namespace
{
[[maybe_unused]] auto deadzonef = deadzone<float>;
[[maybe_unused]] auto deadzoned = deadzone<double>;
} // namespace

inline float wrap_float(float x, float low, float high)
{
  // already in range
  if (low <= x && x < high)
  {
    return x;
  }

  const auto range     = high - low;
  const auto inv_range = float(1) / range; // should evaluate at compile time, multiplies below at runtime
  const auto num_wraps = floor((x - low) * inv_range);
  return x - range * num_wraps;
}

inline float wrap_pi(float x)
{
  return wrap_float(x, -M_PI, M_PI);
}

inline float smoothstepf(float x)
{
  const float x2 = x * x;
  return 3.0f * x2 - 2.0f * x2 * x;
}

template<class T> inline int get_min_element_index(const T array[], int arrSize)
{
  int   min_element_index = 0;
  float min_element       = array[0];
  for (int i = 1; i < arrSize; i++)
  {
    if (array[i] < min_element)
    {
      min_element       = array[i];
      min_element_index = i;
    };
  };
  return min_element_index;
}

} // namespace math
