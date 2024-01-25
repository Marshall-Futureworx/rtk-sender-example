# rtk-sender-example
Example that connects to a GPS and sends RTCM RTK data via MAVLink using MAVSDK.

In the event that instability still exists within the Sees.ai RTK sender example, download the Sees.ai fork of QGroundControl and set it to run on start-up through systemd and connecting to Microhard serial port ONLY.

## Prerequisites

1. Install [MAVSDK](https://github.com/mavlink/MAVSDK) on your system.
2. Install `cmake` and a compiler such as `GCC`.
3. Get the git submodule: `git submodule update --init --recursive`.

## Build

```
cmake -Bbuild -H.
cmake --build build
```
**Note:** before building the "rtk-sender-example.cpp" file needs to be modified to comment out hard-coded GPS positional fix of Lavant test site and uncomment the auto survey code. 

**Recommend:** use survey duration of 180s to match defaults within Qgroundcontrol.

## Run

Connect GPS over serial, find the serial device, as well as baudrate.
Also find the MAVSDK connection URL to connect to the vehicle using MAVLink.

```
usage: build/rtk-sender-example <serial device> <baudrate> <mavlink connection>

e.g.: build/rtk-sender-example /dev/ttyUSB0 38400 udp://:14550

```
Note: use baudrate 0 to determine baudrate automatically
