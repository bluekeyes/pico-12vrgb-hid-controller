use csscolorparser::Color;
use std::{error, fmt};

#[cfg_attr(unix, path = "device/unix.rs")]
#[cfg_attr(windows, path = "device/windows.rs")]
mod backend;

pub const DEFAULT_VENDOR_ID: u16 = 0xCAFE;
pub const DEFAULT_PRODUCT_ID: u16 = 0x4100;

pub const LAMP_COUNT: u8 = 4;
pub const MULTI_UPDATE_LAMP_COUNT: usize = 4;

pub mod hid {
    pub mod report {
        pub const LAMP_ARRAY_MULTI_UPDATE: u16 = 0x04;
        pub const LAMP_ARRAY_RANGE_UPDATE: u16 = 0x05;
        pub const LAMP_ARRAY_CONTROL: u16 = 0x06;
        pub const VENDOR_12VRGB_RESET: u16 = 0x30;
    }

    pub mod usage_page {
        pub const LIGHTING: u16 = 0x59;
        pub const VENDOR: u16 = 0xFF00;
    }

    pub mod usage {
        pub const LIGHTING_LAMP_ARRAY: u16 = 0x01;
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

    pub fn read_temperature(&self) -> Result<i16, Error> {
        self.d.read_temperature()
    }
}

pub enum Report {
    Reset(ResetReport),
    LampArrayMultiUpdate(LampArrayMultiUpdateReport),
    LampArrayRangeUpdate(LampArrayRangeUpdateReport),
    LampArrayControl(LampArrayControlReport),
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

#[derive(Debug, Copy, Clone)]
pub struct RGBI {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub i: u8,
}

impl RGBI {
    pub fn zero() -> Self {
        RGBI {
            r: 0,
            g: 0,
            b: 0,
            i: 0,
        }
    }
}

impl From<&RGBI> for [u8; 4] {
    fn from(rgbi: &RGBI) -> Self {
        [rgbi.r, rgbi.g, rgbi.b, rgbi.i]
    }
}

impl From<&Color> for RGBI {
    fn from(value: &Color) -> Self {
        let (r, g, b, _) = value.to_linear_rgba_u8();
        RGBI { r, g, b, i: 1 }
    }
}

pub struct LampArrayMultiUpdateReport {
    pub flags: u16,
    pub count: u8,
    pub lamp_ids: [u8; MULTI_UPDATE_LAMP_COUNT],
    pub colors: [RGBI; MULTI_UPDATE_LAMP_COUNT],
}

pub struct LampArrayRangeUpdateReport {
    pub flags: u16,
    pub lamp_id_start: u8,
    pub lamp_id_end: u8,
    pub color: RGBI,
}

pub struct LampArrayControlReport {
    pub autonomous: bool,
}
