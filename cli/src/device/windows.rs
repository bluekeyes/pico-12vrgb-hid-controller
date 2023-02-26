use crate::device::{hid, Error, Report, SetAnimationMode};
use std::sync;
use windows::core::HSTRING;
use windows::Devices::Enumeration::DeviceInformation;
use windows::Devices::HumanInterfaceDevice::{HidDevice, HidFeatureReport, HidOutputReport};
use windows::Storage::FileAccessMode;
use windows::Storage::Streams::{ByteOrder, DataWriter, IBuffer};
use windows::Win32::Devices::Sensors::{self, ISensor, ISensorManager};
use windows::Win32::System::Com;
use windows::Win32::System::Com::StructuredStorage::PROPVARIANT;
use windows::Win32::UI::Shell::PropertiesSystem::PROPERTYKEY;

pub struct Device {
    vendor: HidDevice,
    lamp_array: HidDevice,
    temp_sensor: ISensor,
}

impl From<windows::core::Error> for Error {
    fn from(value: windows::core::Error) -> Self {
        Self::Backend(Box::new(value))
    }
}

impl Device {
    pub fn open(vendor_id: u16, product_id: u16) -> Result<Device, Error> {
        init_com_once()?;

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

        let device_id = find_first_device(&vendor_filter)?.Id()?;
        let vendor = HidDevice::FromIdAsync(&device_id, FileAccessMode::ReadWrite)?.get()?;

        let device_id = find_first_device(&lamp_array_filter)?.Id()?;
        let lamp_array = HidDevice::FromIdAsync(&device_id, FileAccessMode::ReadWrite)?.get()?;

        let temp_sensor = find_temp_sensor(vendor_id, product_id)?;

        Ok(Device {
            vendor,
            lamp_array,
            temp_sensor,
        })
    }

    pub fn send_report(&self, report: Report) -> Result<(), Error> {
        let report_id = report.id() as u16;
        match report {
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

            Report::Reset(report) => {
                let d = &self.vendor;
                let r = d.CreateFeatureReportById(report_id)?;

                ReportWriter::new(&r)?.write_byte(report.flags())?.close()?;

                d.SendFeatureReportAsync(&r)?.get()?;
                Ok(())
            }

            Report::SetAnimation(mode, report) => {
                let write_report = |r: &dyn HidReport| -> Result<(), Error> {
                    ReportWriter::new(r)?
                        .write_byte(report.lamp_id)?
                        .write_byte(report.animation.type_byte())?
                        .write_bytes(&report.animation.data())?
                        .close()
                };

                let d = &self.vendor;
                match mode {
                    SetAnimationMode::Default => {
                        let r = d.CreateFeatureReportById(report_id)?;
                        write_report(&r)?;
                        d.SendFeatureReportAsync(&r)?.get()?;
                    }
                    SetAnimationMode::Current => {
                        let r = d.CreateOutputReportById(report_id)?;
                        write_report(&r)?;
                        d.SendOutputReportAsync(&r)?.get()?;
                    }
                };
                Ok(())
            }
        }
    }

    pub fn read_temperature(&self) -> Result<f64, Error> {
        const TEMPERATURE_PID: u32 = 2;

        unsafe {
            let value = self.temp_sensor.GetData()?.GetSensorValue(&PROPERTYKEY {
                fmtid: Sensors::SENSOR_DATA_TYPE_ENVIRONMENTAL_GUID,
                pid: TEMPERATURE_PID,
            })?;
            Ok(value.Anonymous.Anonymous.Anonymous.fltVal as f64)
        }
    }
}

fn find_first_device(aqs_filter: &HSTRING) -> Result<DeviceInformation, Error> {
    let devices = DeviceInformation::FindAllAsyncAqsFilter(aqs_filter)?.get()?;
    if devices.Size()? < 1 {
        Err(Error::NotFound)
    } else {
        Ok(devices.GetAt(0)?)
    }
}

fn find_temp_sensor(vendor_id: u16, product_id: u16) -> Result<ISensor, Error> {
    let path_prefix = format!("\\\\?\\HID#VID_{vendor_id:04X}&PID_{product_id:04X}&");

    unsafe {
        let sensor_manager: ISensorManager =
            Com::CoCreateInstance(&Sensors::SensorManager, None, Com::CLSCTX_INPROC_SERVER)?;

        let sensors = sensor_manager.GetSensorsByType(&Sensors::GUID_SensorType_Temperature)?;
        for i in 0..sensors.GetCount()? {
            let sensor = sensors.GetAt(i)?;
            let path = sensor.GetProperty(&Sensors::SENSOR_PROPERTY_DEVICE_PATH)?;
            if get_pwsz_string(path)?.starts_with(&path_prefix) {
                return Ok(sensor);
            }
        }
    }

    Err(Error::NotFound)
}

unsafe fn get_pwsz_string(pv: PROPVARIANT) -> Result<String, Error> {
    pv.Anonymous
        .Anonymous
        .Anonymous
        .pwszVal
        .to_string()
        .map_err(|err| Error::Backend(Box::new(err)))
}

static COM_INIT: sync::Once = sync::Once::new();

fn init_com_once() -> Result<(), Error> {
    let mut r: Result<(), Error> = Ok(());
    COM_INIT.call_once(|| unsafe {
        r = Com::CoInitializeEx(None, Com::COINIT_MULTITHREADED).map_err(From::from)
    });
    r
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
