from cobs import cobs
import crcmod
import struct
import types

crc16_func = crcmod.mkCrcFun(0x1011B, initCrc=0, rev=False)

# Dufour telemetry message types.
# https://docs.google.com/document/d/1Q3wXPjhvY55_nOWiSYBC0esl_9sg1q6cHtfZDspMlpw/edit#

VERSION_0_2 = 0x44
VERSION_0_3 = 0x45

MSG_TAG = types.SimpleNamespace()
MSG_TAG.SOURCE_ID = 0x02
MSG_TAG.SEQUENCE = 0x03
MSG_TAG.LOG_SUFFIX = 0x04
MSG_TAG.TIME_EPOCH = 0x11  # 17
MSG_TAG.PILOT_CMD_ACK = 0x20
MSG_TAG.VEHICLE_ARMED = 0x21
MSG_TAG.TELEMETRY_MARKER_BUTTON = 0x26
MSG_TAG.VEHICLE_ANGULAR_RATES = 0x30
MSG_TAG.VEHICLE_ATTITUDE_QUAT = 0x31
MSG_TAG.RATES_SETPOINT = 0x32
MSG_TAG.VEHICLE_ATTITUDE_EULER = 0x33
MSG_TAG.ATTITUDE_EULER_SETPOINT = 0x34
MSG_TAG.ACTUATOR_CMD = 0x40
MSG_TAG.ODOMETRY = 0x42
MSG_TAG.UBX = 0x50
MSG_TAG.NMEA = 0x51
MSG_TAG.GEIGER = 0x52
MSG_TAG.IMU = 0x60
MSG_TAG.GEOFENCE = 0x65
MSG_TAG.SIM_LINK_FORCE_TORQUE = 0x70
MSG_TAG.SIM_ACTUATOR_TORQUE = 0x71
MSG_TAG.SIM_ACTUATOR_VEL_CMD = 0x72
MSG_TAG.SIM_LINK = 0x78
MSG_TAG.SIM_JOINT = 0x79


ACTUATOR = types.SimpleNamespace()
ACTUATOR.WING_TILT_ABS = 0x01 # 1
ACTUATOR.WING_TILT_REL = 0x02 # 2
ACTUATOR.PROP_LEFT_1 = 0x11 #(inner)
ACTUATOR.PROP_LEFT_2 = 0x15 #(outer)
ACTUATOR.PROP_RIGHT_1 = 0x21 # 33 (inner)
ACTUATOR.PROP_RIGHT_2 = 0x25 # 37 (outer)
ACTUATOR.PROP_PITCH_DOWN = 0x31 # 49
ACTUATOR.PROP_PITCH_UP = 0x39 # 57
ACTUATOR.FLAPPERON_LEFT_1 = 0x41 # 65
ACTUATOR.FLAPPERON_LEFT_2 = 0x45 # 69
ACTUATOR.FLAPPERON_RIGHT_1 = 0x51 # 81
ACTUATOR.FLAPPERON_RIGHT_2 = 0x55 # 85
ACTUATOR.STABILATOR = ACTUATOR.ELEVATOR_LEFT = 0x61 # 97 
ACTUATOR.ELEVATOR_RIGHT = 0x65 # 101
ACTUATOR.RUDDER = 0x71 # 113

IMU_FLAG = types.SimpleNamespace()
IMU_FLAG.GYRO_PRESENT = 0x01
IMU_FLAG.ACCEL_PRESENT = 0x02
IMU_FLAG.BARO_PRESENT = 0x04

# Links are rigid bodies in the physics simulation.
LINK = types.SimpleNamespace()
LINK.PROPS_LEFT = 11
LINK.PROP_2 = 12
LINK.PROP_3 = 13
LINK.PROP_4 = 14
LINK.PROP_TAIL = 31
LINK.FUSELAGE = 51
LINK.WING = 61
LINK.STABILATOR = 64

# Joints connect a pair of links.
JOINT = types.SimpleNamespace()
JOINT.PROP_1 = 11
JOINT.PROP_2 = 12
JOINT.PROP_3 = 13
JOINT.PROP_4 = 14
JOINT.PROP_TAIL = 31
JOINT.AILERON_1 = 41
JOINT.AILERON_2 = 42
JOINT.AILERON_3 = 43
JOINT.AILERON_4 = 44
JOINT.STABILATOR = 51
JOINT.WING = 61


def serialize_msg(tag, body):
    serialized_msg = struct.pack('<B', tag)
    serialized_msg += body
    serialized_msg += struct.pack('>H', crc16_func(serialized_msg))
    return cobs.encode(serialized_msg) + b'\x00'


# This is a protocol-agnostic utility for parsing byte arrays.
class BufferDecoder:
    def __init__(self, buffer):
        self.buffer = buffer
        self.cursor = 0

    def skip(self, length):
        if self.cursor + length > len(self.buffer):
            raise IndexError()
        self.cursor += length

    def unpack(self, format):
        length = struct.calcsize(format)
        if self.cursor + length > len(self.buffer):
            raise IndexError()
        tuple = struct.unpack_from(format, self.buffer, self.cursor)
        self.cursor += length
        return tuple

    def get_chunk(self, length):
        if self.cursor + length > len(self.buffer):
            raise IndexError()
        chunk = self.buffer[slice(self.cursor, self.cursor + length)]
        self.cursor += length
        return chunk

    def has_more(self):
        return self.cursor < len(self.buffer)
