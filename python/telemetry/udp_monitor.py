import asyncio
from cobs import cobs
import crcmod
import socket
import telem

from protocol_udp import UdpProtocol


crc16_func = crcmod.mkCrcFun(0x1011b, initCrc=0, rev=False)


async def main():

  loop = asyncio.get_running_loop()
  terminate = loop.create_future()

  BIND_ADDR = "0.0.0.0" # "10.42.0.1"
  INTERFACE_ADDR = "0.0.0.0" # "10.42.0.1"
  SOCKET_RX_PORT = 3931

  def create_multicast_socket():
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    sock.bind((BIND_ADDR, SOCKET_RX_PORT))

    def add_membership(addr):
      mreq = socket.inet_aton(addr) + socket.inet_aton(INTERFACE_ADDR)
      sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    add_membership("224.3.39.31")
    add_membership("224.3.39.32")
    add_membership("224.3.39.33")
    add_membership("224.3.39.34")

    return sock

  udp_transport, udp_protocol = await loop.create_datagram_endpoint(
    lambda: UdpProtocol(terminate, None),
    sock=create_multicast_socket())

  def on_udp_frame(frame_cobs):
    msg = f"FRAME[{len(frame_cobs)}]:"

    for msg_cobs in frame_cobs.split(b'\00'):
      msg_raw = cobs.decode(msg_cobs)
      if len(msg_raw) >= 3:
        msg_tag = msg_raw[0]
        msg += f" {msg_tag}[{len(msg_raw)}]"
        if msg_tag == telem.MSG_TAG.SOURCE_ID:
          source = msg_raw[1:-2].decode()
          msg += f" (source={source})"
        if crc16_func(msg_raw) != 0:
          msg += " CRC!"

    print(msg)

  udp_protocol.on_frame = on_udp_frame

  try:
    await terminate

  finally:
    udp_transport.close()
    loop.stop()
    loop.close()


# logging.basicConfig(level=logging.DEBUG)
asyncio.run(main(), debug=True)
