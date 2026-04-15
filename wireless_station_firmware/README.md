# Wireless Station Firmware



## 🚀 Load firmware into the Pico 2 W
The process to load a program into the Pico 2 W board is straightforward,
simply execute [this bash file](scripts/upload_wireless_station_firmware.sh)
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



## 🌡️ Generate icons for the OLED display
1. From the firmware's root folder, run [this script](scripts/generate_icon_bytemap.sh)
```
./scripts/generate_icon_bytemap.sh
```



## 🧪 Testing
[Ceedling](https://github.com/throwtheswitch/ceedling) is used to
run the test suite for this project. To install this tool, run:
```
gem install ceedling
```

To run the tests for the wireless station firmware, go to the
[tests](tests/) folder and run:
```
ceedling clean
ceedling test:all
```
