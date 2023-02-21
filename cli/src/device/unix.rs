use crate::device::{Error, Report};

pub struct Device {}

impl Device {
    pub fn open(vendor_id: u16, product_id: u16) -> Result<Device, Error> {
        unimplemented!();
    }

    pub fn send_report(&self, report: Report) -> Result<(), Error> {
        unimplemented!();
    }
}
