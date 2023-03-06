import serial
import str_commands
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('angle_deg', default='0.0')
args = parser.parse_args()

ser = serial.Serial('/dev/ttyACM0',  115200) # open serial port

str_commands.set_angle(
    ser, 
    float(args.angle_deg),
)

ser.close()                         # close port