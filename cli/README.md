# pico-12vrgb-ctrl

A program to control the USB device from the command line. See the help output
for details on the supported commands:

```
Usage: pico-12vrgb-ctrl.exe [OPTIONS] <COMMAND>

Commands:
  lamp-array       Control lights directly
  set-animation    Manage built-in animations
  reset            Reset the controller hardware
  get-temperature  Read the internal temperature sensor
  help             Print this message or the help of the given subcommand(s)

Options:
      --vendor-id <ID>   Override the USB vendor ID for the device
      --product-id <ID>  Override the USB product ID for the device
  -h, --help             Print help
  -V, --version          Print version
```

## Platform Support

The CLI is structured to support multiple platforms, but currently only
implements a Windows backend. Other backends might be added if there's a need
for them.

### Windows

On Windows, the CLI uses WinRT and Win32 APIs (via the [`windows`
crate][windows-rs]) to interact with the device. Partly due to API limitations
and partly for simplicity, the CLI sends reports in the exact binary format
expected by the device. This means it only works with the firmware from this
repository and that the CLI version and the firmware version must match.

The main reason to use Windows APIs directly instead of an abstraction library
like `hidapi` is that sensor devices are only accessible via the Win32 Sensors
API and are hidden from the normal HID APIs.

The CLI uses exact binary report formats because the
`Windows.Devices.HumanInterfaceDevice` API provides no way I could find to set
repeated report items by HID usage. I think this limitation does not exist in
the lower-level Win32 HID API, but the WinRT API is easier to use and has nicer
Rust bindings.

Currently, all parts of the `lamp-array` subcommand exit immediately. Similar
to sensors, once a HID device is detected as as a more specific subclass,
Windows hides it from the generic HID APIs. The `Windows.Devices.Lights` API
does not map one-to-one with the underlying HID messages and so isn't a direct
replacement. It also has limitations on when apps can use this API to control
the device (e.g. they must be in the foreground.)

The [Dynamic Lighting][] UI is a reasonable substitute for most of the
`lamp-array` subcommmands.

[windows-rs]: https://crates.io/crates/windows
[Dynamic Lighting]: https://support.microsoft.com/en-us/windows/control-your-dynamic-lighting-devices-in-windows-8e8f22e3-e820-476c-8f9d-9ffc7b6ffcd2

## Development

This is a standard Rust project that builds with `cargo`. There are no
requirements apart from a working Rust installation.

From this directory:

```
# compile the binary, output is in the `target/debug` directory
$ cargo build

# complie and run the binary, with subcommands and flags after the "--"
$ cargo run -- set-animation --help
```
