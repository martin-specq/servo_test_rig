import asyncio
from cobs import cobs
import crcmod
import logging
import os
import socket
import sys
import telem
import websockets

from protocol_udp import UdpProtocol


WEBSOCKET_URI = (
    os.environ.get("WEBSOCKET_URI") or "wss://telem.dufour.aero/stream/cm298y4c/d03"
)

crc16_func = crcmod.mkCrcFun(0x1011B, initCrc=0, rev=False)


async def main():

    loop = asyncio.get_running_loop()
    terminate = loop.create_future()

    BIND_ADDR = "0.0.0.0"  # "10.42.0.1"
    MULTICAST_ADDR = "224.3.39.31"
    INTERFACE_ADDR = "0.0.0.0"  # "10.42.0.1"
    SOCKET_RX_PORT = 3931

    def create_multicast_socket():
        sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
        sock.bind((BIND_ADDR, SOCKET_RX_PORT))

        mreq = socket.inet_aton(MULTICAST_ADDR) + socket.inet_aton(INTERFACE_ADDR)
        sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

        return sock

    udp_transport, udp_protocol = await loop.create_datagram_endpoint(
        lambda: UdpProtocol(terminate, ("224.3.39.32", 3931)),
        sock=create_multicast_socket(),
    )

    connections = {}
    frame_counter_by_size = {}

    def on_udp_frame(frame_cobs):
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

    udp_protocol.on_frame = on_udp_frame

    try:
        await terminate

    finally:
        udp_transport.close()
        loop.stop()
        loop.close()


# logging.basicConfig(level=logging.DEBUG)
asyncio.run(main(), debug=True)
