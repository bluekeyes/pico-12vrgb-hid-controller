use crate::device::{hid, Error, Report};
use windows::core::HSTRING;
use windows::Devices::{Enumeration::DeviceInformation, HumanInterfaceDevice::HidDevice};
use windows::Storage::FileAccessMode;

pub struct Device {
    vendor: HidDevice,
}

impl From<windows::core::Error> for Error {
    fn from(value: windows::core::Error) -> Self {
        Self::Backend(Box::new(value))
    }
}

impl Device {
    pub fn open(vendor_id: u16, product_id: u16) -> Result<Device, Error> {
        let vendor_filter = HidDevice::GetDeviceSelectorVidPid(
            hid::usage_page::VENDOR,
            hid::usage::VENDOR_12VRGB_CONTROLLER,
            vendor_id,
            product_id,
        )?;

        let vendor_id = get_device_from_filter(&vendor_filter)?.Id()?;
        let vendor = HidDevice::FromIdAsync(&vendor_id, FileAccessMode::ReadWrite)?.get()?;

        Ok(Device { vendor })
    }

    pub fn send_report(&self, report: Report) -> Result<(), Error> {
        match report {
            Report::Reset(reset) => {
                let r = self.vendor.CreateFeatureReportById(hid::report::VENDOR_12VRGB_RESET)?;

                // TODO(bkeyes): the data value seems to always be the size of
                // the largest report of the particular type. If using the
                // control API will not work (e.g. due to repeated fields with a
                // single usage), the code will have to pad out the buffer to
                // match the expected lenght

                /*
                let data = DataWriter::new()?;
                data.WriteByte(hid::report::VENDOR_12VRGB_RESET as u8)?;
                data.WriteByte(reset.flags())?;
                data.WriteBytes(&vec![0; 61])?;
                r.SetData(&data.DetachBuffer()?)?;
                */

                let flags = r.GetNumericControl(hid::usage_page::VENDOR, hid::usage::VENDOR_12VRGB_RESET_FLAGS)?;
                flags.SetValue(reset.flags() as i64)?;

                match self.vendor.SendFeatureReportAsync(&r)?.get() {
                    Ok(_) => Ok(()),
                    Err(err) => Err(From::from(err)),
                }
            }
        }
    }
}

fn get_device_from_filter(aqs: &HSTRING) -> Result<DeviceInformation, Error> {
    let devices = DeviceInformation::FindAllAsyncAqsFilter(aqs)?.get()?;
    if devices.Size()? < 1 {
        Err(Error::NotFound)
    } else {
        Ok(devices.GetAt(0)?)
    }
}
