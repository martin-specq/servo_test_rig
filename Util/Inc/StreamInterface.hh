#pragma once

#include <stddef.h>
#include <stdint.h>

class StreamInterface
{
    // Returns the number of bytes available in the read buffer.
  public:
    virtual size_t available() const = 0;

    // Returns the next byte from the read buffer.
  public:
    virtual uint8_t read() = 0;

    // Writes a buffer of bytes, returning the number of bytes written, which may be less than len.
  public:
    virtual size_t write(const uint8_t *buf, size_t len) = 0;
};
