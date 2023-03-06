import serial
import struct

FRAME_HEADER = 0xAB

CMD_NO_CMD                      = 0x00
CMD_SERVO_STOP                	= 0x01
CMD_SERVO_SET_ANGLE             = 0x02
CMD_SERVO_START_SIN             = 0x03
CMD_SERVO_START_TRAP            = 0x04
CMD_SERVO_START_SIN_SWEEP       = 0x05
CMD_SERVO_START_TRAP_SWEEP      = 0x06

def calculate_checksum(msg: bytes):
    return sum(msg) & 0xFF


def stop(ser: serial.Serial):
    checksum = calculate_checksum(struct.pack("<B", CMD_SERVO_STOP))
    frame = struct.pack("<BBB", FRAME_HEADER, CMD_SERVO_STOP, checksum)
    ser.write(frame)

def set_angle(ser: serial.Serial, angle_deg: float):
    msg_body = struct.pack("<Bf", CMD_SERVO_SET_ANGLE, angle_deg)
    checksum = calculate_checksum(msg_body)
    frame = struct.pack("<BBfB", FRAME_HEADER, CMD_SERVO_SET_ANGLE, angle_deg, checksum)
    ser.write(frame)

def start_sinusoidal(ser: serial.Serial, angle_min_deg: float, angle_max_deg: float, period_s: float):
    checksum = calculate_checksum(struct.pack("<Bfff", CMD_SERVO_START_SIN, angle_min_deg, angle_max_deg, period_s))
    frame = struct.pack("<BBfffB", FRAME_HEADER, CMD_SERVO_START_SIN, angle_min_deg, angle_max_deg, period_s, checksum)
    ser.write(frame)

def start_sinusoidal_sweep(ser: serial.Serial, 
                           angle_min_deg: float, 
                           angle_max_deg: float, 
                           period_min_s: float, 
                           period_max_s: float,
                           n_periods: int,
                           n_cycles_per_period: int):
    
    checksum = calculate_checksum(struct.pack(
        "<Bffffii", 
        CMD_SERVO_START_SIN_SWEEP, 
        angle_min_deg, 
        angle_max_deg, 
        period_min_s,
        period_max_s,
        n_periods,
        n_cycles_per_period)
    )
    frame = struct.pack(
        "<BBffffiiB", 
        FRAME_HEADER, 
        CMD_SERVO_START_SIN_SWEEP, 
        angle_min_deg, 
        angle_max_deg, 
        period_min_s,
        period_max_s,
        n_periods,
        n_cycles_per_period, 
        checksum
    )
    ser.write(frame)

def start_trapezoidal(ser: serial.Serial, angle_min_deg: float, angle_max_deg: float, period_s: float, plateau_time_s):
    checksum = calculate_checksum(struct.pack("<Bffff", CMD_SERVO_START_TRAP, angle_min_deg, angle_max_deg, period_s, plateau_time_s))
    frame = struct.pack("<BBffffB", FRAME_HEADER, CMD_SERVO_START_TRAP, angle_min_deg, angle_max_deg, period_s, plateau_time_s, checksum)
    ser.write(frame)
