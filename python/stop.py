import serial
import str_commands
import argparse
import config

ser = serial.Serial(config.USB_DEV_CMD, config.BAUDRATE) # open serial port

str_commands.stop(ser)

ser.close()    
