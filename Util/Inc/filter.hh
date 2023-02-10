/*
 * filter.hh
 *
 *  Created on: Feb 9, 2023
 *      Author: martin
 */

#ifndef INC_FILTER_HH_
#define INC_FILTER_HH_

template<class T, size_t N> class Filter
{
private:
  T 		_buf[N] = {0};
  size_t 	_head = 0;

public:
  void update(T last_value)
  {
    _buf[_head] = last_value;
    _head = (_head + 1) % N;
  }

  T mean_filter(void)
  {
    T sum = 0;
    for(size_t i=0; i<N; i++)
    {
      sum += _buf[i];
    }
    return (T)(sum / N);
  }

  T min_filter(void)
  {
    T min = _buf[0];
    for(size_t i=0; i<N; i++)
    {
      if(_buf[i] < min)
      {
	min = _buf[i];
      }
    }
    return min;
  }
};


#endif /* INC_FILTER_HH_ */
