#pragma once

#include "Config/Types.hh"

class TimeSourceInterface
{
  public:
    virtual uint32_t now_micros() const = 0;
};

class ADCInterface
{
public:
	virtual bool available(void) const = 0;
public:
	virtual void start_conversion(uint8_t *buf, size_t len) = 0;
};
