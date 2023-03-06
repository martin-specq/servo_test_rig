
class UdpProtocol:
  def __init__(self, on_con_lost, send_addr):
    self.on_frame = None
    self.on_con_lost = on_con_lost
    self.transport = None
    self.send_addr = send_addr
    self.paused = False
    print("UDP initialized")

  def connection_made(self, transport):
    self.transport = transport
    print("UDP connected")

  def datagram_received(self, data, addr):
    self.on_frame(data)

  def error_received(self, exc):
    print("UDP connection error:")
    print(exc)
    self.on_con_lost.set_result(True)

  def pause_writing(self):
    self.paused = True
    print("UDP connection paused")

  def resume_writing(self):
    self.paused = False
    print("UDP connection resumed")

  def connection_lost(self, exc):
    print("UDP connection lost")
    if not self.on_con_lost.done():
      self.on_con_lost.set_result(True)

  def send_frame(self, frame):
    if not self.paused:
      # print(frame.hex() + '\n')
      self.transport.sendto(frame, self.send_addr)
