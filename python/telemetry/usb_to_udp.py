
import argparse
import asyncio
from cobs import cobs
import crcmod
from datetime import datetime, timedelta
import serial_asyncio
import socket
import struct
import telem

from protocol_udp import UdpProtocol

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
  parser.add_argument('--device', default='/dev/ttyACM0')
  parser.add_argument('--baudrate', default=115200)
  args = parser.parse_args()

  loop = asyncio.get_running_loop()
  terminate = loop.create_future()

  # BIND_ADDR = "0.0.0.0" # "10.42.0.1"
  BIND_ADDR = "224.3.39.32"
  MULTICAST_ADDR = "224.3.39.31"  # LLFC to HLFC
  MULTICAST_ADDR_RX = "224.3.39.32"  # HLFC to LLFC
  INTERFACE_ADDR = "0.0.0.0" # "10.42.0.1"
  SOCKET_PORT = 3931
  SOCKET_RX_PORT = 3931

  sequence_micros = 0
  frame_buffer = bytearray()

  def create_multicast_socket():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    sock.bind((BIND_ADDR, SOCKET_RX_PORT))

    def add_membership(addr):
      mreq = socket.inet_aton(addr) + socket.inet_aton(INTERFACE_ADDR)
      sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    # add_membership(MULTICAST_ADDR)
    add_membership(MULTICAST_ADDR_RX)

    return sock

  udp_transport, udp_protocol = await loop.create_datagram_endpoint(
    lambda: UdpProtocol(terminate, (MULTICAST_ADDR, SOCKET_PORT)),
    sock=create_multicast_socket())

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
        udp_protocol.send_frame(frame_buffer)

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
  write_serial_frame = lambda frame: serial_protocol.send_frame(frame)

  def on_udp_frame(frame_cobs):
    # print("UDP Message Receifed")
    # msg = f"FRAME[{len(frame_cobs)}]:"

    # for msg_cobs in frame_cobs.split(b'\00'):
    #  msg_raw = cobs.decode(msg_cobs)
    #  if len(msg_raw) >= 3:
    #    msg_tag = msg_raw[0]
    #    msg += f" {msg_tag}[{len(msg_raw)}]"
    #    if msg_tag == telem.MSG_TAG.SOURCE_ID:
    #      source = msg_raw[1:-2].decode()
    #      msg += f" (source={source})"
    #    if crc16_func(msg_raw) != 0:
    #      msg += " CRC!"

    # print(msg)

    # frame_cobs = cobs.encode(msg_raw) + bytes([0])
    write_serial_frame(frame_cobs)

  udp_protocol.on_frame = on_udp_frame

  try:
    await terminate

  finally:
    udp_transport.close()
    serial_transport.close()


asyncio.run(main())
