use csscolorparser::Color;
use std::{error, fmt, io::Write};

#[cfg_attr(unix, path = "device/unix.rs")]
#[cfg_attr(windows, path = "device/windows.rs")]
mod backend;

pub const DEFAULT_VENDOR_ID: u16 = 0xCAFE;
pub const DEFAULT_PRODUCT_ID: u16 = 0x4100;

pub mod hid {
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
            Self::Backend(err) => write!(f, "system error: {err}"),
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
    pub const LAMP_COUNT: u8 = 4;

    pub fn open(vendor_id: u16, product_id: u16) -> Result<Device, Error> {
        backend::Device::open(vendor_id, product_id).map(|d| Device { d })
    }

    pub fn send_report(&self, report: Report) -> Result<(), Error> {
        self.d.send_report(report)
    }

    pub fn read_temperature(&self) -> Result<f64, Error> {
        self.d.read_temperature()
    }
}

pub enum Report {
    LampArrayMultiUpdate(LampArrayMultiUpdateReport),
    LampArrayRangeUpdate(LampArrayRangeUpdateReport),
    LampArrayControl(LampArrayControlReport),
    Reset(ResetFlags),
    SetAnimation(SetAnimationMode, SetAnimationReport),
}

impl Report {
    pub fn id(&self) -> u8 {
        match self {
            Self::LampArrayMultiUpdate(_) => 0x04,
            Self::LampArrayRangeUpdate(_) => 0x05,
            Self::LampArrayControl(_) => 0x06,
            Self::Reset(_) => 0x30,
            Self::SetAnimation(_, _) => 0x31,
        }
    }
}

#[derive(Debug)]
pub struct LampArrayMultiUpdateReport {
    pub flags: LampArrayUpdateFlags,
    pub count: u8,
    pub lamp_ids: [u8; LampArrayMultiUpdateReport::MAX_COUNT],
    pub colors: [HIDLampValue; LampArrayMultiUpdateReport::MAX_COUNT],
}

impl LampArrayMultiUpdateReport {
    pub const MAX_COUNT: usize = 4;
}

#[derive(Debug)]
pub struct LampArrayRangeUpdateReport {
    pub flags: LampArrayUpdateFlags,
    pub lamp_id_start: u8,
    pub lamp_id_end: u8,
    pub color: HIDLampValue,
}

#[derive(Debug)]
pub struct LampArrayUpdateFlags {
    pub update_complete: bool,
}

impl From<&LampArrayUpdateFlags> for u16 {
    fn from(value: &LampArrayUpdateFlags) -> Self {
        let mut flags: u16 = 0;
        if value.update_complete {
            flags |= 1 << 0;
        }
        flags
    }
}

#[derive(Debug)]
pub struct LampArrayControlReport {
    pub autonomous: bool,
}

#[derive(Debug)]
pub struct ResetFlags {
    pub bootsel: bool,
    pub clear_flash: bool,
}

impl From<&ResetFlags> for u8 {
    fn from(value: &ResetFlags) -> Self {
        let mut flags: u8 = 0;
        if value.bootsel {
            flags |= 1 << 0;
        }
        if value.clear_flash {
            flags |= 1 << 1;
        }
        flags
    }
}

#[derive(Debug)]
pub enum SetAnimationMode {
    Default,
    Current,
}

#[derive(Debug)]
pub enum Animation {
    None,
    Breathe(BreatheAnimationData),
    Fade(FadeAnimationData),
}

impl Animation {
    pub const DATA_SIZE: usize = 60;

    pub fn type_byte(&self) -> u8 {
        match self {
            Self::None => 0x00,
            Self::Breathe(_) => 0x01,
            Self::Fade(_) => 0x02,
        }
    }

    pub fn data(&self) -> Vec<u8> {
        match self {
            Self::None => Self::write_data(|_| Ok(())),

            Self::Breathe(ref data) => Self::write_data(|b| {
                b.write_all(&<[u8; 3]>::from(&data.on_color))?;
                b.write_all(&<[u8; 3]>::from(&data.off_color))?;
                b.write_all(&data.on_fade_time_ms.to_le_bytes())?;
                b.write_all(&data.on_time_ms.to_le_bytes())?;
                b.write_all(&data.off_fade_time_ms.to_le_bytes())?;
                b.write_all(&data.off_time_ms.to_le_bytes())
            }),

            Self::Fade(ref data) => Self::write_data(|b| {
                let colors: Vec<u8> = data.colors.iter().flat_map(<[u8; 3]>::from).collect();
                b.write_all(&data.color_count.to_le_bytes())?;
                b.write_all(&colors)?;
                b.write_all(&data.fade_time_ms.to_le_bytes())?;
                b.write_all(&data.hold_time_ms.to_le_bytes())
            }),
        }
    }

    fn write_data<F>(f: F) -> Vec<u8>
    where
        F: Fn(&mut Vec<u8>) -> std::io::Result<()>,
    {
        let mut b: Vec<u8> = Vec::with_capacity(Self::DATA_SIZE);
        f(&mut b)
            .map(|_| b)
            .expect("impl std::io::Write for std::vec::Vec should not return Err")
    }
}

#[derive(Debug)]
pub struct BreatheAnimationData {
    pub on_color: LinearRGB,
    pub off_color: LinearRGB,
    pub on_fade_time_ms: u16,
    pub on_time_ms: u16,
    pub off_fade_time_ms: u16,
    pub off_time_ms: u16,
}

#[derive(Debug)]
pub struct FadeAnimationData {
    pub color_count: u8,
    pub colors: [LinearRGB; FadeAnimationData::MAX_COLORS],
    pub fade_time_ms: u16,
    pub hold_time_ms: u16,
}

impl FadeAnimationData {
    pub const MAX_COLORS: usize = 8;
}

#[derive(Debug)]
pub struct SetAnimationReport {
    pub lamp_id: u8,
    pub animation: Animation,
}

#[derive(Debug, Copy, Clone)]
pub struct HIDLampValue {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub i: u8,
}

impl HIDLampValue {
    pub fn zero() -> Self {
        HIDLampValue {
            r: 0,
            g: 0,
            b: 0,
            i: 0,
        }
    }
}

impl From<&HIDLampValue> for [u8; 4] {
    fn from(rgbi: &HIDLampValue) -> Self {
        [rgbi.r, rgbi.g, rgbi.b, rgbi.i]
    }
}

impl From<&Color> for HIDLampValue {
    fn from(value: &Color) -> Self {
        let [r, g, b, _] = value.to_rgba8();
        HIDLampValue { r, g, b, i: 1 }
    }
}

#[derive(Debug, Copy, Clone)]
pub struct LinearRGB {
    pub r: u8,
    pub g: u8,
    pub b: u8,
}

impl LinearRGB {
    pub fn zero() -> Self {
        LinearRGB { r: 0, g: 0, b: 0 }
    }
}

impl From<&LinearRGB> for [u8; 3] {
    fn from(rgb: &LinearRGB) -> Self {
        [rgb.r, rgb.g, rgb.b]
    }
}

impl From<&Color> for LinearRGB {
    fn from(value: &Color) -> Self {
        let (r, g, b, _) = value.to_linear_rgba_u8();
        LinearRGB { r, g, b }
    }
}
