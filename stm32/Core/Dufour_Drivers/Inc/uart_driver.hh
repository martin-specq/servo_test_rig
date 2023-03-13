#pragma once

#include "main.h"
#include <algorithm>

#include "../../../App/Inc/CircularBuffer.hh"
#include "../../../App/Inc/Math.hh"
#include "../../../App/Inc/StreamInterface.hh"

class UartDriver : public StreamInterface
{
  private:
    UART_HandleTypeDef *_huartx;
  private:
    CircularBuffer<uint8_t, 300> _read_buf;
  private:
    uint8_t _read_byte;
  private:
    CircularBuffer<uint8_t, 600> _write_buf;
  private:
    uint8_t _write_chunk[30];
  private:
    bool _is_writing = false;

  private:
    const uint32_t WriteTimeout = 1000;
  private:
    const uint32_t ReadTimeout = 1000;

  public:
    UartDriver(UART_HandleTypeDef *huartx) : _huartx(huartx) {}

  private:
    void write_raw(const uint8_t *TxBuffer, size_t Size)
    {
      if (HAL_UART_Transmit_IT(_huartx, (uint8_t *)TxBuffer, Size) != HAL_OK)
      {
        Error_Handler();
      }
    }

  public:
    void start()
    {
      if (HAL_UART_Receive_DMA(_huartx, &_read_byte, 1) != HAL_OK)
      {
        Error_Handler();
      }
    }

  public:
    void dma_stop()
    {
      if (HAL_UART_DMAStop(_huartx) != HAL_OK)
      {
        Error_Handler();
      }
    }

  public:
    void stop()
    {
      if (HAL_UART_DeInit(_huartx) != HAL_OK)
      {
        Error_Handler();
      }
    }

  public:
    void restart()
    {
      __HAL_UART_SEND_REQ(_huartx, UART_RXDATA_FLUSH_REQUEST);
      if (HAL_UART_Receive_DMA(_huartx, &_read_byte, 1) != HAL_OK)
      {
        // Error_Handler();
      }
    }

  public:
    uint32_t get_error()
    {
      return HAL_UART_GetError(_huartx);
    }

  public:
    void invert_tx_activelevel()
    {
      if (_huartx->AdvancedInit.AdvFeatureInit == UART_ADVFEATURE_TXINVERT_INIT
          && _huartx->AdvancedInit.TxPinLevelInvert == UART_ADVFEATURE_TXINV_ENABLE)
        return;

      __HAL_UART_DISABLE(_huartx);

      _huartx->AdvancedInit.AdvFeatureInit   = UART_ADVFEATURE_TXINVERT_INIT;
      _huartx->AdvancedInit.TxPinLevelInvert = UART_ADVFEATURE_TXINV_ENABLE;

      UART_AdvFeatureConfig(_huartx);

      __HAL_UART_ENABLE(_huartx);
    }

  public:
    uint8_t serial_tx_active()
    {
      return ((HAL_UART_GetState(_huartx) & HAL_UART_STATE_BUSY_TX) == HAL_UART_STATE_BUSY_TX);
    }

  public:
    USART_TypeDef *get_instance()
    {
      return _huartx->Instance;
    }

  public:
    void on_rx_completed()
    {
      _read_buf.put(_read_byte);
    }

  public:
    void on_tx_completed()
    {
      write_next_chunk();
    }

  public:
    void write_next_chunk()
    {
      size_t next_chunk_size = math::min(sizeof(_write_chunk), _write_buf.size());

      if (next_chunk_size == 0)
      {
        _is_writing = false;
        return;
      }

      // Fetch the data from the circular buffer into a linear chunk buffer.
      // TODO: Optimize CircularBuffer for bulk gets.
      /*for (size_t i = 0; i < next_chunk_size; ++i)
      {
        _write_chunk[i] = _write_buf.get();
      }*/

      _write_buf.get_bulk(_write_chunk, next_chunk_size);

      _is_writing = true;

      write_raw(_write_chunk, next_chunk_size);
    }

    // StreamInterface implementation.

  public:
    size_t available() const override
    {
      return _read_buf.size();
    }

  public:
    uint8_t read() override
    {
      return _read_buf.get();
    }

  public:
    size_t write(const uint8_t *buf, size_t len) override
    {
      if (_write_buf.available() < len)
      {
        // There's not enough space to enqueue this message.
        return 0;
      }

      // Old: Optimize CircularBuffer for bulk puts.
      /*for (size_t i = 0; i < len; ++i)
      {
        _write_buf.put(buf[i]);
      }*/

      _write_buf.put_bulk(const_cast<uint8_t *>(buf), len);

      if (!_is_writing)
      {
        // TODO: Optimize this case. No need for the first chunk to go through the circular buffer.
        write_next_chunk();
      }

      return len;
    }
};
