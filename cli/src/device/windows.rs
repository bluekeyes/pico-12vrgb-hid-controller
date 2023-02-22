use crate::device::{hid, Error, Report};
use windows::core::HSTRING;
use windows::Devices::Enumeration::DeviceInformation;
use windows::Devices::HumanInterfaceDevice::{HidDevice, HidFeatureReport};
use windows::Storage::FileAccessMode;
use windows::Storage::Streams::{ByteOrder, DataWriter, IBuffer};

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
                let r = self
                    .vendor
                    .CreateFeatureReportById(hid::report::VENDOR_12VRGB_RESET)?;

                ReportWriter::new(&r)?.write_byte(reset.flags())?.close()?;

                self.vendor.SendFeatureReportAsync(&r)?.get()?;
                Ok(())
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

trait HidReport {
    fn report_id(&self) -> Result<u16, Error>;
    fn report_length(&self) -> Result<u32, Error>;
    fn set_data(&self, data: &IBuffer) -> Result<(), Error>;
}

impl HidReport for HidFeatureReport {
    fn report_id(&self) -> Result<u16, Error> {
        self.Id().map_err(From::from)
    }

    fn report_length(&self) -> Result<u32, Error> {
        self.Data()?.Length().map_err(From::from)
    }

    fn set_data(&self, data: &IBuffer) -> Result<(), Error> {
        self.SetData(data).map_err(From::from)
    }
}

struct ReportWriter<'a> {
    report: &'a dyn HidReport,
    data: DataWriter,
    length: u32,
}

impl<'a> ReportWriter<'a> {
    fn new(report: &dyn HidReport) -> Result<ReportWriter, Error> {
        let data = DataWriter::new()?;
        data.SetByteOrder(ByteOrder::LittleEndian)?;
        data.WriteByte(report.report_id()? as u8)?;

        Ok(ReportWriter {
            report,
            data,
            length: 1,
        })
    }

    fn write_byte(mut self, value: u8) -> Result<Self, Error> {
        self.data.WriteByte(value)?;
        self.length += 1;
        Ok(self)
    }

    fn close(self) -> Result<(), Error> {
        let padding = self.report.report_length()? - self.length;
        if padding > 0 {
            self.data.WriteBytes(&vec![0; padding as usize])?;
        }
        self.report
            .set_data(&self.data.DetachBuffer()?)
            .map_err(From::from)
    }
}
