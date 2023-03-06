import serial
import str_commands
import argparse

ser = serial.Serial('/dev/ttyACM0',  115200) # open serial port

str_commands.stop(ser)

ser.close()    
