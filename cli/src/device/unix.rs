use crate::device::Report;
use std::error::Error;

pub struct Device {}

impl Device {
    pub fn find() -> Result<Option<Device>, Box<dyn Error>> {
        unimplemented!();
    }

    pub fn send_report(&self, report: Report) -> Result<(), Box<dyn Error>> {
        unimplemented!();
    }
}
