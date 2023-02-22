use std::{error, fmt};

#[cfg_attr(unix, path = "device/unix.rs")]
#[cfg_attr(windows, path = "device/windows.rs")]
mod backend;

pub const DEFAULT_VENDOR_ID: u16 = 0xCAFE;
pub const DEFAULT_PRODUCT_ID: u16 = 0x4100;

pub mod hid {
    pub mod report {
        pub const VENDOR_12VRGB_RESET: u16 = 0x30;
    }

    pub mod usage_page {
        pub const VENDOR: u16 = 0xFF00;
    }

    pub mod usage {
        pub const VENDOR_12VRGB_CONTROLLER: u16 = 0x01;
    }
}

#[derive(Debug)]
pub enum Error {
    /// Indicates that the device is not connected to the system or otherwise
    /// could not be found.
    NotFound,

    /// Wraps an error from the platform-specific USB backend.
    Backend(Box<dyn error::Error>),
}

impl fmt::Display for Error {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        match self {
            Self::NotFound => write!(f, "device not found"),
            Self::Backend(err) => write!(f, "system error: {}", err.to_string()),
        }
    }
}

impl error::Error for Error {
    fn cause(&self) -> Option<&dyn error::Error> {
        match self {
            Self::NotFound => None,
            Self::Backend(err) => Some(err.as_ref()),
        }
    }
}

pub struct Device {
    d: backend::Device,
}

impl Device {
    pub fn open(vendor_id: u16, product_id: u16) -> Result<Device, Error> {
        backend::Device::open(vendor_id, product_id).map(|d| Device { d })
    }

    pub fn send_report(&self, report: Report) -> Result<(), Error> {
        self.d.send_report(report)
    }
}

pub enum Report {
    Reset(ResetReport),
}

pub struct ResetReport {
    pub bootsel: bool,
    pub clear_flash: bool,
}

impl ResetReport {
    /// Returns the value of the VENDOR_12VRGB_RESET_FLAGS report item
    pub fn flags(&self) -> u8 {
        const BOOTSEL: u8 = 1 << 0;
        const CLEAR_FLASH: u8 = 1 << 1;

        let mut flags: u8 = 0;
        if self.bootsel {
            flags |= BOOTSEL;
        }
        if self.clear_flash {
            flags |= CLEAR_FLASH;
        }
        flags
    }
}
