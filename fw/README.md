# pico-12vrgb-controller

This directory contains a Pico SDK project for the controller firmware. The
Raspberry Pi Pico SDK and `cmake` are required.

## Building

```
export PICO_SDK_PATH=/path/to/pico-sdk-1.5.0
mkdir build
cd build
cmake ..
make
```

Edit `src/config.h` before building to modify device configuration.
