import serial
import str_commands
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('angle_min_deg', default='-30.0')
parser.add_argument('angle_max_deg', default='30.0')
parser.add_argument('period_s', default='2.0')
args = parser.parse_args()

ser = serial.Serial('/dev/ttyACM0',  115200) # open serial port

str_commands.start_sinusoidal(
    ser, 
    float(args.angle_min_deg), 
    float(args.angle_max_deg), 
    float(args.period_s)
)

ser.close()                         # close port