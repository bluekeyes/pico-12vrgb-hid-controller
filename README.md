# pico-12vrgb-hid-controller

A 4-channel 12V-G-R-B controller for RGB fans and LED strips using the
Raspberry Pi Pico and controlled via USB HID. Designed to be installed inside a
PC case as an alternative or addition to motherboard headers.

It implements the "Lighting and Illumination" HID usage page spec and a custom
a vendor page for additional control.

Note this does *not* support 3-pin ARGB LEDs, but uses the semi-standard 4-pin
12V-G-R-B connector, where all devices connected to a channel show the same
color.

### Development Status

Software is functional but incomplete. Hardware design is complete but untested
and may change once I have some physical parts.
