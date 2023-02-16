#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

#include "Math.hh"

// Circular buffer of capacity N-1.
template<class T, size_t N> class CircularBuffer
{

  private:
    T      buf_[N];
    size_t head_ = 0;
    size_t tail_ = 0;

  public:
    void put(T item)
    {
      buf_[head_] = item;

      if (full())
      {
        tail_ = (tail_ + 1) % N;
      }

      head_ = (head_ + 1) % N;
    }

    uint8_t put_bulk(T *items, size_t item_size)
    {
      uint8_t buf_overflow = 0;

      // Number of bytes to write at head
      size_t size_head = math::min(math::min(item_size, N - head_), capacity());

      // Number of bytes to write at the beginning of the buffer. 
      // Make sure it does not write more bytes than the buffer capacity
      size_t size_begin = math::min(capacity() - size_head, item_size - size_head);

      // Write data at head
      memcpy((void *)&buf_[head_], (void *)items, size_head);

      // Write data at the beginning of buf
      if (size_begin > 0)
      {
        memcpy((void *)&buf_[0], (void *)&items[size_head], size_begin);
      }

      // Specify whether the end of buf is reached 
      if (size_head == N - head_)
      {
        buf_overflow = 1;
      }

      // Store previous head
      size_t prev_head = head_;

      // Update head
      head_ = (head_ + size_head + size_begin) % N;

      // In case head_ overtakes tail_, update tail_
      if ((tail_ > prev_head && (tail_ <= head_ || buf_overflow)) || (tail_ <= head_ && buf_overflow))
      {
        tail_ = (head_ + 1) % N;
      }

      // Return number of written bytes
      return size_head + size_begin;
    }

    T get()
    {
      if (empty())
      {
        return T();
      }

      const T data = buf_[tail_];
      tail_        = (tail_ + 1) % N;

      return data;
    }

    size_t get_bulk(T *items, size_t item_size)
    {
      const size_t buf_size = size();

      // Number of bytes to read from the tail 
      size_t size_tail = math::min(math::min(item_size, N - tail_), buf_size);

      // Number of bytes to read from the beginning of the buffer
      size_t size_begin = math::min(buf_size - size_tail, item_size - size_tail);

      // Get data from tail_
      memcpy((void *)items, (void*)&buf_[tail_], size_tail);

      if (size_begin > 0)
      {
        // Get data from the beginning of the buffer
        memcpy((void *)&items[size_tail], (void *)buf_, size_begin);
      }

      // Update tail_
      tail_ = (tail_ + size_tail + size_begin) % N;

      // Return number of read bytes
      return size_tail + size_begin;
    }

    void reset()
    {
      head_ = tail_;
    }

    bool empty() const
    {
      return (head_ == tail_);
    }

    bool full() const
    {
      return ((head_ + 1) % N) == tail_;
    }

    size_t capacity() const
    {
      return N - 1;
    }

    size_t size() const
    {
      size_t size = N;

      if (head_ >= tail_)
      {
        size = head_ - tail_;
      }
      else
      {
        size = N + head_ - tail_;
      }

      return size;
    }

    size_t available() const
    {
      return N - 1 - size();
    }
};
