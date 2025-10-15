# Wireless Station Firmware


## 🚀 Load firmware into the Pico 2 W
The process to load a program into the Pico 2 W board is straightforward,
simply execute [this bash file](upload_wireless_station_firmware.sh)
and the new version will be uploaded to the board.


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
Unit tests should be run with the Ceedling tool from the
[tests](tests/) folder.
```
cd tests
ceedling clean
ceedling test:all
```