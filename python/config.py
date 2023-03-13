import sys

if sys.platform == 'win32':
    # Don't forget to setup the COM port splitter before writing both port names
    USB_DEV_TELEM = "COM1"
    USB_DEV_CMD = "COM2"
else:
    USB_DEV_TELEM = "/dev/ttyUSB0"
    USB_DEV_CMD = "/dev/ttyUSB0"

BAUDRATE = 115200
