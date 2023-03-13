The test rig sends telemetry over the UART-to-USB adapter COM port and also receives commands 
from the user to command the servo via the same COM port. Since Windows does not allow several 
applications to share a single COM port, a COM port splitter software must be to used to create 
one virtual COM port for the telemetry and a different virtual COM port for the user commands. 
To setup both virtual COM ports, do the following:
    1. Install the HDD Virtual Serial Port Tool by executing "free-virtual-serial-port-tool.exe".
    2. Launch "Virtual Serial Port Tool"
    3. In the "Splitted Ports" window, click on "Split Ports"
    4. 
