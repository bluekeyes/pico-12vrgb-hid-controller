# pico-12vrgb-hid-controller

A 4-channel 12V-G-R-B controller for RGB fans and LED strips using the Raspberry
Pi Pico and controlled via USB HID. Designed to be installed inside a PC case as
an alternative or addition to motherboard headers.

It implements the "Lighting and Illumination" (`0x59`) HID usage page, the
"Sensors" (`0x20`) HID usage page (for a temperature sensor), and a vendor
(`0xFF00`) HID usage page for custom controls.

Note this does *not* support 3-pin addressable RGB LEDs, but uses the
semi-standard 4-pin 12V-G-R-B connector, where all devices connected to a
channel show the same color.

## Structure

This project is split into three parts:

* `hw/` contains schematics and PCB files (KiCAD)
* `fw/` contains device firmware (C)
* `cli/` contains the command-line control program (Rust)

See each directory for more details.

## Development Status

Firmware and the control program are functional and mostly complete. The
hardware design is complete but untested and may change once I have some
physical parts.
