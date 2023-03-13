
#include "../../App/Inc/IntervalWaiter.hh"

namespace dfr
{

IntervalWaiter::IntervalWaiter(const TimeSourceInterface *time_source, unsigned long interval_micros)
  : time_source(time_source), interval_micros(interval_micros), now_micros(0), last_micros(0)
{}

bool IntervalWaiter::next_interval()
{
  now_micros = time_source->now_micros();
  if (now_micros - last_micros >= interval_micros)
  {
    do
    {
      last_micros += interval_micros;
    } while (now_micros - last_micros >= interval_micros);
    return true;
  }
  else
  {
    return false;
  }
}

void IntervalWaiter::wait()
{
  while (!next_interval())
    ;
}

} // namespace dfr
