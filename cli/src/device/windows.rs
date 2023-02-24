use crate::device::{hid, Error, Report};
use windows::core::HSTRING;
use windows::Devices::Enumeration::DeviceInformation;
use windows::Devices::HumanInterfaceDevice::{HidDevice, HidFeatureReport, HidOutputReport};
use windows::Storage::FileAccessMode;
use windows::Storage::Streams::{ByteOrder, DataWriter, IBuffer};
use windows::Win32::Devices::Sensors::{self, ISensorManager};
use windows::Win32::System::Com;
use windows::Win32::UI::Shell::PropertiesSystem::PROPERTYKEY;

pub struct Device {
    vendor: HidDevice,
    lamp_array: HidDevice,
}

impl From<windows::core::Error> for Error {
    fn from(value: windows::core::Error) -> Self {
        Self::Backend(Box::new(value))
    }
}

impl Device {
    pub fn open(vendor_id: u16, product_id: u16) -> Result<Device, Error> {
        unsafe {
            // Initialize COM thread for later use
            Com::CoInitializeEx(None, Com::COINIT_MULTITHREADED)?;
        }

        let vendor_filter = HidDevice::GetDeviceSelectorVidPid(
            hid::usage_page::VENDOR,
            hid::usage::VENDOR_12VRGB_CONTROLLER,
            vendor_id,
            product_id,
        )?;

        let lamp_array_filter = HidDevice::GetDeviceSelectorVidPid(
            hid::usage_page::LIGHTING,
            hid::usage::LIGHTING_LAMP_ARRAY,
            vendor_id,
            product_id,
        )?;

        let vendor_id = get_device_from_filter(&vendor_filter)?.Id()?;
        let vendor = HidDevice::FromIdAsync(&vendor_id, FileAccessMode::ReadWrite)?.get()?;

        let lamp_arry_id = get_device_from_filter(&lamp_array_filter)?.Id()?;
        let lamp_array = HidDevice::FromIdAsync(&lamp_arry_id, FileAccessMode::ReadWrite)?.get()?;

        Ok(Device { vendor, lamp_array })
    }

    pub fn send_report(&self, report: Report) -> Result<(), Error> {
        let report_id = report.id() as u16;
        match report {
            Report::Reset(report) => {
                let d = &self.vendor;
                let r = d.CreateFeatureReportById(report_id)?;

                ReportWriter::new(&r)?.write_byte(report.flags())?.close()?;

                d.SendFeatureReportAsync(&r)?.get()?;
                Ok(())
            }

            Report::LampArrayMultiUpdate(report) => {
                let d = &self.lamp_array;
                let r = d.CreateOutputReportById(report_id)?;

                let colors: Vec<u8> = report.colors.iter().flat_map(<[u8; 4]>::from).collect();
                ReportWriter::new(&r)?
                    .write_byte(report.count)?
                    .write_u16(report.flags)?
                    .write_bytes(&report.lamp_ids)?
                    .write_bytes(&colors)?
                    .close()?;

                d.SendOutputReportAsync(&r)?.get()?;
                Ok(())
            }

            Report::LampArrayRangeUpdate(report) => {
                let d = &self.lamp_array;
                let r = d.CreateOutputReportById(report_id)?;

                ReportWriter::new(&r)?
                    .write_u16(report.flags)?
                    .write_byte(report.lamp_id_start)?
                    .write_byte(report.lamp_id_end)?
                    .write_bytes(&<[u8; 4]>::from(&report.color))?
                    .close()?;

                d.SendOutputReportAsync(&r)?.get()?;
                Ok(())
            }

            Report::LampArrayControl(report) => {
                let d = &self.lamp_array;
                let r = d.CreateFeatureReportById(report_id)?;

                ReportWriter::new(&r)?
                    .write_byte(if report.autonomous { 1 } else { 0 })?
                    .close()?;

                d.SendFeatureReportAsync(&r)?.get()?;
                Ok(())
            }
        }
    }

    pub fn read_temperature(&self) -> Result<i16, Error> {
        const TEMPERATURE_PID: u32 = 2;

        unsafe {
            let sensor_manager: ISensorManager =
                Com::CoCreateInstance(&Sensors::SensorManager, None, Com::CLSCTX_INPROC_SERVER)?;

            let sensors = sensor_manager.GetSensorsByType(&Sensors::GUID_SensorType_Temperature)?;
            let sensor = if sensors.GetCount()? > 0 {
                // TODO(bkeyes): figure out how to get our sensor, not just the first one...
                sensors.GetAt(0)?
            } else {
                return Err(Error::NotFound);
            };

            let value = sensor.GetData()?.GetSensorValue(&PROPERTYKEY {
                fmtid: Sensors::SENSOR_DATA_TYPE_ENVIRONMENTAL_GUID,
                pid: TEMPERATURE_PID,
            })?;

            Ok((100.0 * value.Anonymous.Anonymous.Anonymous.fltVal) as i16)
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

impl HidReport for HidOutputReport {
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

    fn write_bytes(mut self, value: &[u8]) -> Result<Self, Error> {
        self.data.WriteBytes(value)?;
        self.length += value.len() as u32;
        Ok(self)
    }

    fn write_u16(mut self, value: u16) -> Result<Self, Error> {
        self.data.WriteUInt16(value)?;
        self.length += 2;
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
