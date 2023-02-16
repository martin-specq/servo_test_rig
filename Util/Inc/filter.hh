/*
 * filter.hh
 *
 *  Created on: Feb 9, 2023
 *      Author: martin
 */

#ifndef INC_FILTER_HH_
#define INC_FILTER_HH_

#define FILTER_BUFFER_SIZE 100

template<class T> class Filter
{
private:
	T _buf[FILTER_BUFFER_SIZE] = {0};
	size_t _head = 0;

public:
	void update(T last_value)
	{
		_buf[_head] = last_value;
		_head = (_head + 1) % FILTER_BUFFER_SIZE;
	}

	T apply_mean(size_t window_size)
	{
		T sum = 0;
		if(window_size > FILTER_BUFFER_SIZE)
		{
			window_size = FILTER_BUFFER_SIZE;
		}

		if(window_size > _head)
		{
			for(size_t i = 0; i < _head; i++)
			{
				sum += _buf[i];
			}
			for(size_t i = FILTER_BUFFER_SIZE - window_size + _head; i < FILTER_BUFFER_SIZE; i++)
			{
				sum += _buf[i];
			}
		}
		else
		{
			for(size_t i = _head - window_size; i < _head; i++)
			{
				sum += _buf[i];
			}
		}

		return (T)(sum / window_size);
	}

	/**T apply_min(size_t window_size)
	{
		T min = _buf[0];
		for(size_t i = 0; i < N; i++)
		{
			if(_buf[i] < min)
			{
				min = _buf[i];
			}
		}
		return min;
	}*/
};

#endif /* INC_FILTER_HH_ */
