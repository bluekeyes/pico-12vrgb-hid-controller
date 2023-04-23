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

![The assembled pico-12vrgb-hid-controller PCB](/img/pcb.jpg)

![The assembled PCB installed in the 3D-printed case](/img/case.jpg)

## Animations

When running in autonomous mode, the controller implements configurable
animations. Use the CLI to set the parameters or save an animation as the
default for a channel.

### Breathe

Fades a single color on and off. The fade on time, on time, fade off time, and
off time are all configurable.

https://user-images.githubusercontent.com/1745813/233868169-5f5d1403-0b08-4b99-b51f-30fe5f666057.mp4

### Fade

Fades between up to 8 different colors. The fade time and hold time are
configurable.

https://user-images.githubusercontent.com/1745813/233868393-662b7a13-106e-483d-9052-4a47977f7780.mp4

## Project Structure

This project is split into three parts:

* `hw/` contains schematics and PCB files (KiCAD)
* `fw/` contains device firmware (C)
* `cli/` contains the command-line control program (Rust)

See each directory for more details.

## Limitations

The hardware is only tested with Windows 11 and the included CLI. The CLI does
not yet support other platforms. The firmware has not been verified with other
implementations of the HID Lighting and Illumination spec.

## License

MIT
