use std::error::Error;

#[cfg_attr(unix, path = "device/unix.rs")]
#[cfg_attr(windows, path = "device/windows.rs")]
mod sys;

pub struct Device {
    d: sys::Device,
}

impl Device {
    pub fn find() -> Result<Option<Device>, Box<dyn Error>> {
        sys::Device::find().map(|opt_d| opt_d.map(|d| Device { d: d }))
    }

    pub fn send_report(&self, report: Report) -> Result<(), Box<dyn Error>> {
        self.d.send_report(report)
    }
}

pub enum Report {
    Reset,
}
