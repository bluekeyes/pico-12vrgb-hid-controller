use clap::{self, Args, Parser, Subcommand};
use std::ops::RangeInclusive;
use std::{thread, time::Duration};

use crate::device::{self, Device, Report};
use crate::temperature;

mod animation;
mod lamparray;

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
pub struct Root {
    #[command(subcommand)]
    pub command: Commands,

    /// Override the USB vendor ID for the device
    #[arg(long, value_name = "ID", global = true)]
    pub vendor_id: Option<u16>,

    /// Override the USB product ID for the device
    #[arg(long, value_name = "ID", global = true)]
    pub product_id: Option<u16>,
}

impl Root {
    pub fn run(&self) -> Result<(), Box<dyn std::error::Error>> {
        let vid = self.vendor_id.unwrap_or(device::DEFAULT_VENDOR_ID);
        let pid = self.product_id.unwrap_or(device::DEFAULT_PRODUCT_ID);

        let dev = Device::open(vid, pid)?;

        match &self.command {
            Commands::LampArray { command } => command.run(&dev),

            Commands::SetAnimation { animation } => animation.run(&dev),

            Commands::Reset(args) => dev
                .send_report(Report::Reset(device::ResetFlags {
                    bootsel: args.bootsel,
                    clear_flash: args.clear_flash,
                }))
                .map_err(From::from),

            Commands::GetTemperature(args) => loop {
                println!("{:.2}", args.units.from_celsius(dev.read_temperature()?));
                match args.interval {
                    Some(interval) => thread::sleep(Duration::from_secs(interval as u64)),
                    None => return Ok(()),
                }
            },
        }
    }
}

#[derive(Subcommand)]
pub enum Commands {
    /// Control lights directly
    LampArray {
        #[command(subcommand)]
        command: lamparray::Command,
    },

    /// Manage built-in animations
    #[command(subcommand_value_name("TYPE"))]
    #[command(subcommand_help_heading("Animation Types"))]
    #[command(disable_help_subcommand(true))]
    SetAnimation {
        #[command(subcommand)]
        animation: animation::Animation,
    },

    /// Reset the controller hardware
    Reset(ResetArgs),

    /// Read the internal temperature sensor
    GetTemperature(GetTemperatureArgs),
}

#[derive(Args)]
pub struct ResetArgs {
    /// Reset into the mass-storage interface for programming
    ///
    /// This is equivalent to reseting the Pi Pico while holding the BOOTSEL button.
    #[arg(long)]
    bootsel: bool,

    /// Clear all saved settings in flash
    #[arg(long)]
    clear_flash: bool,
}

#[derive(Args)]
pub struct GetTemperatureArgs {
    /// The units to use when printing the temperature
    #[arg(short, long, default_value_t = temperature::Degrees::Celsius)]
    #[arg(value_parser = temperature::Degrees::parse)]
    units: temperature::Degrees,

    /// How often to print the temperature, in seconds. If unset, print the current temperature
    /// and exit.
    interval: Option<u32>,
}

/// Parses a 1-indexed lamp/channel string to 0-indexed lamp ID
fn lamp_id_parser(value: &str) -> Result<u8, String> {
    const RANGE: RangeInclusive<usize> = 1..=(Device::LAMP_COUNT as usize);

    let id: usize = value.parse().map_err(|_| "invalid lamp number")?;
    if RANGE.contains(&id) {
        Ok((id - 1) as u8)
    } else {
        Err(format!(
            "lamp number must be in range {}-{}",
            RANGE.start(),
            RANGE.end()
        ))
    }
}
