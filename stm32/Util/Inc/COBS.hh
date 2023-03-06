
#include <stddef.h>
#include <stdint.h>

// https://en.wikipedia.org/wiki/Consistent_Overhead_Byte_Stuffing

/*
 * EncodeCOBS byte stuffs "length" bytes of data
 * at the location pointed to by "ptr", writing
 * the output to the location pointed to by "dst".
 *
 * Returns the length of the encoded data.
 */

inline size_t EncodeCOBS(const uint8_t *ptr, size_t length, uint8_t *dst)
{
  uint8_t *start = dst;
  uint8_t  code = 1, *code_ptr = dst++; /* Where to insert the leading count */

  while (length--)
  {
    if (*ptr) /* Input byte not zero */
      *dst++ = *ptr, ++code;

    if (!*ptr++ || code == 0xFF) /* Input is zero or complete block */
      *code_ptr = code, code = 1, code_ptr = dst++;
  }
  *code_ptr = code; /* Final code */

  return dst - start;
}

/*
 * DecodeCOBS decodes "length" bytes of data at
 * the location pointed to by "ptr", writing the
 * output to the location pointed to by "dst".
 *
 * Returns the length of the decoded data
 * (which is guaranteed to be <= length).
 */
inline size_t DecodeCOBS(const uint8_t *ptr, size_t length, uint8_t *dst)
{
  const uint8_t *start = dst, *end = ptr + length;
  uint8_t        code = 0xFF, copy = 0, input_invalid = 0;

  for (; ptr < end; copy--)
  {
    if (copy != 0)
    {
      if (*ptr != 0)
      {
        *dst++ = *ptr++;
      }
      else
      {
        input_invalid = 1;
        break; /* zero byte in cobs block detected */
      }
    }
    else
    {
      if (code != 0xFF)
        *dst++ = 0;
      copy = code = *ptr++;
      if (code == 0)
      {
        input_invalid = 1;
        break; /* zero byte in code byte detected */
      }
      if (ptr + code - 1 > end && code != 1)
      {
        input_invalid = 1;
        break; /* block longer than remaining input */
      }
    }
  }
  return ((input_invalid) ? 0 : dst - start);
}
