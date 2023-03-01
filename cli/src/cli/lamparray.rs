use clap::{Args, CommandFactory, Subcommand};
use csscolorparser::{self, Color};

use crate::cli;
use crate::device::{self, Device, Report};

#[derive(Subcommand)]
pub enum Command {
    /// Set lamp array controls
    SetControl(SetControlArgs),

    /// Set lamp colors
    Update(UpdateArgs),

    /// Set a range of lamps to a color
    UpdateRange(UpdateRangeArgs),
}

impl Command {
    pub fn run(&self, dev: &Device) -> Result<(), Box<dyn std::error::Error>> {
        match self {
            Self::SetControl(args) => {
                if let Some(autonomous) = args.autonomous {
                    dev.send_report(Report::LampArrayControl(device::LampArrayControlReport {
                        autonomous,
                    }))
                    .map_err(From::from)
                } else {
                    Ok(())
                }
            }

            Self::Update(args) => {
                const MAX_COUNT: usize = device::LampArrayMultiUpdateReport::MAX_COUNT;

                let count = args.lamp_ids.len();
                if count != args.colors.len() {
                    let mut err = cli::Root::command();
                    err.error(
                        clap::error::ErrorKind::ArgumentConflict,
                        "The number of colors must match the number of lamps",
                    )
                    .exit();
                }
                if count > MAX_COUNT {
                    let mut err = cli::Root::command();
                    err.error(
                        clap::error::ErrorKind::TooManyValues,
                        format!("The number of lamps must be at most {}", MAX_COUNT),
                    )
                    .exit();
                }

                let mut lamp_ids = [0u8; MAX_COUNT];
                lamp_ids[0..count].copy_from_slice(&args.lamp_ids);

                let mut colors = [device::HIDLampValue::zero(); MAX_COUNT];
                colors[0..count].copy_from_slice(
                    &args
                        .colors
                        .iter()
                        .map(<device::HIDLampValue>::from)
                        .collect::<Vec<_>>(),
                );

                dev.send_report(Report::LampArrayMultiUpdate(
                    device::LampArrayMultiUpdateReport {
                        flags: device::LampArrayUpdateFlags {
                            update_complete: true,
                        },
                        count: count as u8,
                        lamp_ids,
                        colors,
                    },
                ))
                .map_err(From::from)
            }

            Self::UpdateRange(args) => dev
                .send_report(Report::LampArrayRangeUpdate(
                    device::LampArrayRangeUpdateReport {
                        flags: device::LampArrayUpdateFlags {
                            update_complete: true,
                        },
                        lamp_id_start: args.lamp_id_start.unwrap_or(0),
                        lamp_id_end: args.lamp_id_end.unwrap_or(Device::LAMP_COUNT - 1),
                        color: args
                            .color
                            .as_ref()
                            .map(From::from)
                            .unwrap_or(device::HIDLampValue::zero()),
                    },
                ))
                .map_err(From::from),
        }
    }
}

#[derive(Args)]
pub struct SetControlArgs {
    /// Enable or disable autonomous mode
    #[arg(long)]
    pub autonomous: Option<bool>,
}

#[derive(Args)]
pub struct UpdateArgs {
    /// The lamp ID to update; repeat to update more than one lamp. The number of lamps must
    /// match the number of colors.
    #[arg(long = "lamp", value_name = "ID")]
    #[arg(value_parser = cli::lamp_id_parser)]
    pub lamp_ids: Vec<u8>,

    /// The new color of the lamp; repeat to update more than one lamp. The number of colors
    /// must match the number of lamps.
    ///
    /// Colors are specified as CSS color strings. The alpha channel is ignored. To turn
    /// off a lamp, set the color to black (#000000).
    #[arg(long = "color", value_name = "COLOR")]
    #[arg(value_parser = csscolorparser::parse)]
    pub colors: Vec<Color>,
}

#[derive(Args)]
pub struct UpdateRangeArgs {
    /// The first lamp in the update range. If unset, use the first lamp on the device.
    #[arg(long = "start", value_name = "ID")]
    #[arg(value_parser = cli::lamp_id_parser)]
    pub lamp_id_start: Option<u8>,

    /// The last lamp in the update range. If unset, use the last lamp on the device.
    #[arg(long = "end", value_name = "ID")]
    #[arg(value_parser = cli::lamp_id_parser)]
    pub lamp_id_end: Option<u8>,

    /// The new color of the lamps. If unset, turn off the lamps in the range.
    ///
    /// Colors are specified as CSS color strings. The alpha channel is ignored. To turn
    /// off a lamp, set the color to black (#000000).
    #[arg(long)]
    #[arg(value_parser = csscolorparser::parse)]
    pub color: Option<Color>,
}
