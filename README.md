# pico-12vrgb-hid-controller

A 4-pin 12V-G-R-B controller (for RGB fans and LED strips) using the Raspberry
Pi Pico / RP2040 and controlled via USB HID. Designed to be installed inside a
PC case as an alternative or addition to motherboard headers.

It implements the "Lighting and Illumination" page of the USB HID spec and a
custom a vendor page for additional control.

Note this does *not* support 3-pin ARGB LEDs, but uses the semi-standard 4-pin
12V-G-R-B connector, where all devices connected to a channel show the same
color.
