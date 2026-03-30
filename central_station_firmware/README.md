# Central Station Firmware



## 🚀 Load firmware into the Pico 2 W
The process to load a program into the Pico 2 W board is straightforward.

1. Edit and run [this script](set_secrets.sh) in the current shell using 
the ```source``` command:
```
source set_secrets.sh
```
This creates the environment variables that will be used by the board as 
the credentials to connect to the WiFi network and establish the MQTT 
connection.

2. From the same terminal, run [this bash file](upload_central_station_firmware.sh) 
as usual to upload the firmware to the board.



## 🖥️ Check the data being transmitted over the USB cable
1. Find how the computer names the board as a character device.
```
ls /dev/cu.*
```
It's usually named _cu.usbmodem1101_ or similar.

2. Run the minicom terminal emulator program to visualize the
data being sent by the board through the USB connection
```
minicom -b 100000 -D /dev/cu.usbmodem1101
```
, where the -b option indicates the baudrate and -D indicates
the device from which to read.



## 🧪 Testing
[Ceedling](https://github.com/throwtheswitch/ceedling) is used to
run the test suite for this project. To install this tool, run:
```
gem install ceedling
```

To run the tests for the central station firmware, go to the
[tests](tests/) folder and run:
```
ceedling clean
ceedling test:all
```
