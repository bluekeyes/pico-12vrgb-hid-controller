use crate::device::{Error, Report};

pub struct Device {}

impl Device {
    pub fn open(_vendor_id: u16, _product_id: u16) -> Result<Device, Error> {
        unimplemented!()
    }

    pub fn send_report(&self, _report: Report) -> Result<(), Error> {
        unimplemented!()
    }

    pub fn read_temperature(&self) -> Result<i16, Error> {
        unimplemented!()
    }
}
