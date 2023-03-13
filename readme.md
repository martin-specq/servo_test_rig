This repository contains the necessary code to operate the X2.3 servo test rig.

# STM32 project
The stm32 folder contains the embedded code to be flashed on the Nucleo-STM32L476RG.
In order to flash the code, do the following:
1. Open the STM32CubeIDE
2. Import the project
   - Click on "File" and then on "Open Project from File System..."
   - Click on "Directory..." and select the stm32 folder
   - Click on "Finish"
3. Build the project
   - From the Project Explorer, right click on the project name and click on "Build"
4. Flash the project
   - From the Project Explorer, right click on the project and click on "Run As" and then on 
     "1 STM32 C/C++ Application". 
   - A window will open. Click on "OK".
       
The flashing should start.

# Telemetry logging

## Windows

### COM port splitter setup

The test rig sends telemetry over the UART-to-USB adapter COM port and also receives commands 
from the user to command the servo via the same COM port. Since Windows does not allow several 
applications to share a single COM port, a COM port splitter software must be to used to create 
one virtual COM port for the telemetry and a different virtual COM port for the user commands. 

To setup both virtual COM ports, do the following:
1. Install the HDD Virtual Serial Port Tool by executing "free-virtual-serial-port-tool.exe" located in the "windows_setup" folder.
2. Launch "Virtual Serial Port Tool"
3. In the "Splitted Ports" window, click on "Split Ports"
4. In the "Select existing device" window, click on the UART-to-USB adapter device name.
5. In the "Split to" window, add "COM1" and "COM2" by using the "+" button.
6. Click on "Create".

![image](https://user-images.githubusercontent.com/114927032/224658344-081d514a-25de-461f-916a-3b1de6ebde2b.png)

If your 2 created virtual COM ports are not 'COM1' and 'COM2', then the 
variables 'USB_DEV_CMD' and 'USB_DEV_TELEM' in "config.py" must be changed accordingly. For instance,
if the 2 created virtual COM ports are 'COM3' and 'COM7', the following lines in "config.py" must be changed as follow:
```python
    if sys.platform == "win32":
        USB_DEV_TELEM = "COM3"
        USB_DEV_CMD = "COM7"
```

Once the COM port has been setup, run the following command to start logging the telemetry on Grafana:

```
python start_logging.py
```

## Linux

Run the following command to start logging the telemetry on Grafana:

```
python start_logging.py
```

If an error such as "could not open port '/dev/ttyUSB0'" terminates the program, it means that 
the UBS device name is not '/dev/ttyUSB0'. In order to find the USB device name, run the following 
command:

```
ls /dev/ttyUSB*
```

The command output should provide the USB device name. Then, update the variables 'USB_DEV_CMD'
and 'USB_DEV_CMD' in config.py. For instance, if the USB device name is "/dev/ttyUSB1", then 
modify "config.py" as follow:

```python
else:
    USB_DEV_TELEM = "/dev/ttyUSB1"
    USB_DEV_CMD = "/dev/ttyUSB1"
```

# Servo control

## Set servo angle

```
python set_angle.py <angle_deg>
```
where:
    - <angle_deg> is the servo target angle in degrees.

## Sinuoidal trajectory with constant period 

```
python start_sinus.py <angle_min_deg> <amgle_max_deg> <period_s>
```
where:
    - <angle_min_deg> is the peak minimum angle in degreea of the sinusoidal trjectory
    - <angle_max_deg> is the peak maximum angle in degrees of the sinusoidal trjectory
    - <period_s> is the period in seconds of the sinusoidal trajectory

## Sinuoidal trajectory with descending period sweep

```
python start_sinus_sweep.py <angle_min_deg> <amgle_max_deg> <per_min_s> <per_max_s> <n_per> <n_cycles_per_per>
```
where:
    - <angle_min_deg> is the peak minimum angle in degreea of the sinusoidal trjectory
    - <angle_max_deg> is the peak maximum angle in degrees of the sinusoidal trjectory
    - <per_min_s> is the lower bound period in seconds of the period sweep
    - <per_max_s> is the higher bound period in seconds of the period sweep
    - <n_per> is the number of periods between <per_min_s> and <per_max_s> on a linear scale
    - <n_cycles_per_per> is the number of cycles for each period

## Stop any sinusoidal trajectory and reset the servo position

```
python stop.py
```




    
