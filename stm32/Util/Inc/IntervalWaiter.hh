#pragma once

#include "DeviceInterfaces.hh"

namespace dfr
{

class IntervalWaiter
{
  private:
    const TimeSourceInterface *time_source;
  private:
    const unsigned long interval_micros;
  private:
    unsigned long now_micros;
  private:
    unsigned long last_micros;

  public:
    IntervalWaiter(const TimeSourceInterface *time_source, unsigned long interval_micros);

  public:
    void wait();
  public:
    bool next_interval();
  public:
    unsigned long get_now_micros()
    {
      return now_micros;
    }
};

} // namespace dfr
