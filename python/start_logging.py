
import argparse
import asyncio
from cobs import cobs
import crcmod
from datetime import datetime, timedelta
import serial_asyncio
import socket
import struct
import telemetry.telem as telem
import websockets
import os

WEBSOCKET_URI = (
    os.environ.get("WEBSOCKET_URI") or "wss://telem.dufour.aero/stream/cm298y4c/d03"
)

crc16_func = crcmod.mkCrcFun(0x1011b, initCrc=0, rev=False)

class SerialProtocol(asyncio.Protocol):
  def __init__(self, serial_closed):
    self.on_msg = lambda frame: None
    self.serial_closed = serial_closed
    self.may_write = True
    self.buffer = bytearray()
    self.synchronized = False
    print("Serial initialized")

  def connection_made(self, transport):
    self.transport = transport
    self.transport.write(bytes([0]))
    print("Serial connected")

  def data_received(self, data):
    for byte in data:
      self.byte_received(byte)

  def byte_received(self, byte):
    # print(str(byte))
    if not self.synchronized:
      self.synchronized = (byte == 0)
    else:
      if byte != 0:
        self.buffer.append(byte)
      else:
        try:
          msg_raw = cobs.decode(self.buffer)
          self.on_msg(self.buffer, msg_raw)
        finally:
          self.buffer.clear()

  def connection_lost(self, exc):
    self.serial_closed.set_result(True)
    print("Serial connection lost")

  def pause_writing(self):
    self.may_write = False
    print("Serial connection paused")

  def resume_writing(self):
    self.may_write = True
    print("Serial connection resumed")

  def send_frame(self, frame):
    if self.may_write:
      self.transport.write(frame)


async def main():

  parser = argparse.ArgumentParser(description='Proxy data between LLFC serial connection and UDP.')
  parser.add_argument('--device', default='COM1')
  parser.add_argument('--baudrate', default=115200)
  args = parser.parse_args()

  loop = asyncio.get_running_loop()
  terminate = loop.create_future()

  sequence_micros = 0
  frame_buffer = bytearray()
  connections = {}
  frame_counter_by_size = {}

  def wss_send_frame(frame_cobs):
    # Downsample data according to frame size.
    frame_size = len(frame_cobs)
    if frame_size not in frame_counter_by_size:
        frame_counter_by_size[frame_size] = 0

    frame_counter_by_size[frame_size] += 1
    if frame_counter_by_size[frame_size] < frame_size // 100:
        return
    frame_counter_by_size[frame_size] = 0

    # Decode the frame.
    source = None
    for msg_cobs in frame_cobs.split(b'\00'):
        msg_raw = cobs.decode(msg_cobs)
        if len(msg_raw) >= 3:
            if crc16_func(msg_raw) != 0:
                print(f"WARNING: message failed CRC check: {msg_raw.hex()}")
            msg_tag = msg_raw[0]
            if msg_tag == telem.MSG_TAG.SOURCE_ID:
                source = msg_raw[1:-2].decode()
                os.environ['SOURCE_ID'] = source

    if not source:
        print("WARNING: Got frame with no source ID. Can not send.")
        return

    async def send_frame():
        try:
            if source not in connections:
                connections[source] = None
                connections[source] = await websockets.connect(
                    WEBSOCKET_URI + "/" + source,
                    ping_interval=5,
                    ping_timeout=5,
                    close_timeout=10,
                )
                print(f"Websocket connected with source \"{source}\"")

            if connections[source]:
                await connections[source].send(frame_cobs)
        except Exception as e:
            del connections[source]
            print(e)
            raise e
    asyncio.create_task(send_frame())

  def on_serial_msg(msg_cobs, msg_raw):
    nonlocal sequence_micros
    nonlocal frame_buffer

    if len(msg_raw) < 3:
      print(f"Error: message too short: {len(msg_raw)} bytes. Can not process.")
      return

    if crc16_func(msg_raw) != 0:
      print("Warning: invalid message CRC. Forwarding it anyway.")
      print(crc16_func(msg_raw), crc16_func(msg_raw[:-2]), struct.unpack('>H', msg_raw[-2:])[0])

    msg_tag, = struct.unpack_from('<B', msg_raw)

    if msg_tag == telem.MSG_TAG.SEQUENCE:
      if sequence_micros > 0:
        time_msg = struct.pack('<BQ', telem.MSG_TAG.TIME_EPOCH, sequence_micros)
        time_msg += struct.pack('>H', crc16_func(time_msg))
        frame_buffer += cobs.encode(time_msg)
        frame_buffer += b'\x00'
        wss_send_frame(frame_buffer)

      frame_buffer.clear()
      sequence_micros = round((datetime.utcnow() - datetime.utcfromtimestamp(0)) / timedelta(microseconds=1))

    frame_buffer += msg_cobs + b'\x00'

  serial_transport, serial_protocol = await serial_asyncio.create_serial_connection(
    loop,
    lambda: SerialProtocol(terminate),
    args.device,
    args.baudrate,
    bytesize=8,
    parity='N',
    stopbits=1)

  serial_protocol.on_msg = on_serial_msg

  try:
    await terminate

  finally:
    serial_transport.close()
    loop.stop()
    loop.close()


asyncio.run(main())
