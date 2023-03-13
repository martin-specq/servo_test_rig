#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>

class BufferWriter
{
  private:
    uint8_t *_buffer;
    size_t   _maxLength;
    size_t   _cursor;

  public:
    BufferWriter(uint8_t *buffer, size_t maxLength) : _buffer(buffer), _maxLength(maxLength), _cursor(0) {}

    size_t length()
    {
      return _cursor;
    }

    template<class T> BufferWriter &operator<<(T value)
    {
      if (_cursor <= _maxLength - sizeof(T))
      {
        *reinterpret_cast<T *>(_buffer + _cursor) = value;
        _cursor += sizeof(T);
      }
      return *this;
    }

    void write(size_t length, const void *value)
    {
      if (_cursor <= _maxLength - length)
      {
        memcpy(_buffer + _cursor, value, length);
        _cursor += length;
      }
    }
};
