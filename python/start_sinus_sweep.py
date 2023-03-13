import serial
import str_commands
import argparse
import config

parser = argparse.ArgumentParser()
parser.add_argument('angle_min_deg', default='-30.0')
parser.add_argument('angle_max_deg', default='30.0')
parser.add_argument('period_min_s', default='1.0')
parser.add_argument('period_max_s', default='10.0')
parser.add_argument('n_periods', default='10')
parser.add_argument('n_cycles_per_per', default='5')
args = parser.parse_args()

ser = serial.Serial(config.USB_DEV_CMD,  config.BAUDRATE) # open serial port

str_commands.start_sinusoidal_sweep(
    ser, 
    float(args.angle_min_deg), 
    float(args.angle_max_deg), 
    float(args.period_min_s),
    float(args.period_max_s),
    int(args.n_periods),
    int(args.n_cycles_per_per)
)

ser.close()                         # close port