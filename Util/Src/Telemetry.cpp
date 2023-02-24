
#include "Telemetry.hh"

#include "COBS.hh"
#include "Math.hh"

namespace telem
{

SerialWriter::SerialWriter(StreamInterface &stream) : stream(stream)
{
  // Start with a COBS framing byte.
  uint8_t framing_byte = 0;
  stream.write(&framing_byte, 1);
}

void SerialWriter::write_message(uint8_t tag, uint8_t length, const void *value)
{
  uint8_t      message_buffer[300];
  BufferWriter bufferWriter(message_buffer, sizeof(message_buffer));
  bufferWriter << tag;
  bufferWriter.write(length, value);

  // CRC calculation.
  const uint16_t crc16 = crc_finalize(crc_update(crc_init(), message_buffer, size_t(length) + 1));
  bufferWriter << reinterpret_cast<const uint8_t *>(&crc16)[1];
  bufferWriter << reinterpret_cast<const uint8_t *>(&crc16)[0];

  uint8_t message_cobs_buffer[sizeof(message_buffer) + 10];
  size_t  message_cobs_length                = EncodeCOBS(message_buffer, bufferWriter.length(), message_cobs_buffer);
  message_cobs_buffer[message_cobs_length++] = 0; // COBS framing byte

  stream.write(message_cobs_buffer, message_cobs_length);

  ++sequence_number;
}

void SerialWriter::write_sequence_message()
{
  write_message(MSG_TAG_SEQUENCE, sequence_msg {VERSION_MARKER_0_3, sequence_number});
}

} // namespace telem
