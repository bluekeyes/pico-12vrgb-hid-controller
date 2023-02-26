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
implements a Windows backend. I might implement other backends might be added if
there's a need for them.

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

I wanted to use the `Windows.Devices.Lights` API to implement the `lamp-array`
subcommand instead of sending low-level reports, but as of February 2023, I
could not get it working. The `LampArray.FromIdAsync` function that serves as
the entry point to the API always hangs for 5 minutes, regardless of the
function inputs or device state. Debugging suggests it never attempts to
communicate with the device and is stuck in a polling loop waiting for some
internal API state to change. This issue [was reported by other people as
well][lamp-array-hang], in that situation causing `explorer.exe` to stop
responding until the timeout. Microsoft fixed this in **KB5017380** by moving
the call out of the main `explorer.exe` thread, instead of by fixing the
underlying `LampArray` API.

As a result, the "Lighting and Illumination" HID implementation is unverified
and I don't know if it complies with the standard yet.

[windows-rs]: https://crates.io/crates/windows
[lamp-array-hang]: https://superuser.com/questions/1642074/trying-to-figure-out-windows-10-login-delay-xperf-shows-explorer-exe-hanging-up

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
