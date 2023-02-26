use clap::{self, Args, CommandFactory, Parser, Subcommand};
use std::ops::RangeInclusive;
use std::{thread, time::Duration};

use crate::device::{self, Device, Report};
use crate::temperature;

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

        let d = Device::open(vid, pid)?;

        match &self.command {
            Commands::LampArray { command } => match command {
                lamparray::Commands::SetControl(args) => {
                    if let Some(autonomous) = args.autonomous {
                        d.send_report(Report::LampArrayControl(device::LampArrayControlReport {
                            autonomous,
                        }))
                        .map_err(From::from)
                    } else {
                        Ok(())
                    }
                }

                lamparray::Commands::Update(args) => {
                    const MAX_COUNT: usize = device::LampArrayMultiUpdateReport::MAX_COUNT;

                    let count = args.lamp_ids.len();
                    if count != args.colors.len() {
                        let mut err = Root::command();
                        err.error(
                            clap::error::ErrorKind::ArgumentConflict,
                            "The number of colors must match the number of lamps",
                        )
                        .exit();
                    }
                    if count > MAX_COUNT {
                        let mut err = Root::command();
                        err.error(
                            clap::error::ErrorKind::TooManyValues,
                            format!("The number of lamps must be at most {}", MAX_COUNT),
                        )
                        .exit();
                    }

                    let mut lamp_ids = [0u8; MAX_COUNT];
                    lamp_ids[0..count].copy_from_slice(&args.lamp_ids);

                    let mut colors = [device::LampValue::zero(); MAX_COUNT];
                    colors[0..count].copy_from_slice(
                        &args
                            .colors
                            .iter()
                            .map(<device::LampValue>::from)
                            .collect::<Vec<_>>(),
                    );

                    d.send_report(Report::LampArrayMultiUpdate(
                        device::LampArrayMultiUpdateReport {
                            count: count as u8,
                            flags: 0x0001, // TODO(bkeyes): make a constant
                            lamp_ids,
                            colors,
                        },
                    ))
                    .map_err(From::from)
                }

                lamparray::Commands::UpdateRange(args) => {
                    d.send_report(Report::LampArrayRangeUpdate(
                        device::LampArrayRangeUpdateReport {
                            flags: 0x0001, // TODO(bkeyes): make a constant
                            lamp_id_start: args.lamp_id_start.unwrap_or(0),
                            lamp_id_end: args.lamp_id_end.unwrap_or(Device::LAMP_COUNT - 1),
                            color: args
                                .color
                                .as_ref()
                                .map(From::from)
                                .unwrap_or(device::LampValue::zero()),
                        },
                    ))
                    .map_err(From::from)
                }
            },

            Commands::SetAnimation { animation } => match animation {
                animation::Animation::Breathe(args) => {
                    const DEFAULT_ON_TIME: animation::Time = animation::Time(500);
                    const DEFAULT_FADE_TIME: animation::Time = animation::Time(1000);
                    const DEFAULT_OFF_TIME: animation::Time = animation::Time(1000);

                    let mode = if args.shared.default {
                        device::SetAnimationMode::Default
                    } else {
                        device::SetAnimationMode::Current
                    };

                    let on_color = &args.on_color;
                    let off_color = args.off_color.as_ref().unwrap_or(on_color);

                    d.send_report(Report::SetAnimation(
                        mode,
                        device::SetAnimationReport {
                            lamp_id: args.shared.lamp_id,
                            animation: device::Animation::Breathe(device::BreatheAnimationData {
                                on_color: on_color.into(),
                                off_color: off_color.into(),
                                on_fade_time_ms: args.on_fade_time.unwrap_or(DEFAULT_FADE_TIME).0,
                                on_time_ms: args.on_time.unwrap_or(DEFAULT_ON_TIME).0,
                                off_fade_time_ms: args.off_fade_time.unwrap_or(DEFAULT_FADE_TIME).0,
                                off_time_ms: args.off_time.unwrap_or(DEFAULT_OFF_TIME).0,
                            }),
                        },
                    ))
                    .map_err(From::from)
                }

                animation::Animation::Fade(args) => {
                    const MAX_COLORS: usize = device::FadeAnimationData::MAX_COLORS;
                    const DEFAULT_FADE_TIME: animation::Time = animation::Time(2000);
                    const DEFAULT_HOLD_TIME: animation::Time = animation::Time(1000);

                    let mode = if args.shared.default {
                        device::SetAnimationMode::Default
                    } else {
                        device::SetAnimationMode::Current
                    };

                    let color_count = args.colors.len();
                    if color_count > MAX_COLORS {
                        let mut err = Root::command();
                        err.error(
                            clap::error::ErrorKind::TooManyValues,
                            format!("The animation can use at most {} colors", MAX_COLORS),
                        )
                        .exit();
                    }

                    let mut colors = [device::RGB::zero(); MAX_COLORS];
                    colors[0..color_count].copy_from_slice(
                        &args
                            .colors
                            .iter()
                            .map(<device::RGB>::from)
                            .collect::<Vec<_>>(),
                    );

                    d.send_report(Report::SetAnimation(
                        mode,
                        device::SetAnimationReport {
                            lamp_id: args.shared.lamp_id,
                            animation: device::Animation::Fade(device::FadeAnimationData {
                                color_count: color_count as u8,
                                colors,
                                fade_time_ms: args.fade_time.unwrap_or(DEFAULT_FADE_TIME).0,
                                hold_time_ms: args.hold_time.unwrap_or(DEFAULT_HOLD_TIME).0,
                            }),
                        },
                    ))
                    .map_err(From::from)
                }
            },

            Commands::Reset(args) => d
                .send_report(Report::Reset(device::ResetReport {
                    bootsel: args.bootsel,
                    clear_flash: args.clear_flash,
                }))
                .map_err(From::from),

            Commands::GetTemperature(args) => loop {
                println!("{:.2}", args.units.convert_raw(d.read_temperature()?));
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
        command: lamparray::Commands,
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

mod lamparray {
    use crate::cli;
    use clap::{Args, Subcommand};
    use csscolorparser::{self, Color};

    #[derive(Subcommand)]
    pub enum Commands {
        /// Set lamp array controls
        SetControl(SetControlArgs),

        /// Set lamp colors
        Update(UpdateArgs),

        /// Set a range of lamps to a color
        UpdateRange(UpdateRangeArgs),
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
}

pub mod animation {
    use std::ops::RangeInclusive;

    use crate::cli;
    use clap::{Args, Subcommand};
    use csscolorparser::{self, Color};

    #[derive(Subcommand)]
    pub enum Animation {
        /// Fade a color on and off
        ///
        /// The "breathe" animation cycles between two colors, the first at its full luminance and the
        /// second at zero luminance. The light intesity changes in four stages:
        ///
        ///
        ///        | A | B | C | D |
        ///        |   |___|   |   |
        ///  light |  /|   |\  |   |
        ///        | / |   | \ |   |
        ///        |/  |   |  \|___|
        ///              time
        ///
        ///   A: On Fade
        ///   B: On
        ///   C: Off Fade
        ///   D: Off
        ///
        /// Each phase can last up to 65 seconds, for a total cycle time of around 4 minutes.
        ///
        /// The second color is optional. If not set, the animation uses the primary color with the
        /// luminance set to zero.
        #[command(verbatim_doc_comment)]
        Breathe(BreatheArgs),

        /// Crossfade between multiple colors
        Fade(FadeArgs),
    }

    #[derive(Args)]
    pub struct BreatheArgs {
        #[command(flatten)]
        pub shared: SharedArgs,

        /// The on color for the animation.
        ///
        /// Colors are specified as CSS color strings. The alpha channel is ignored.
        #[arg(long)]
        #[arg(value_parser = csscolorparser::parse)]
        pub on_color: Color,

        /// The off color for the animation. If unset, use the same color as in the "on" state.
        ///
        /// Colors are specified as CSS color strings. The alpha channel is ignored.
        #[arg(long)]
        #[arg(value_parser = csscolorparser::parse)]
        pub off_color: Option<Color>,

        /// The on fade time (A) in fractional seconds
        #[arg(long, value_name = "SECONDS")]
        #[arg(value_parser = animation_time_parser)]
        pub on_fade_time: Option<Time>,

        /// The on time (B) in fractional seconds
        #[arg(long, value_name = "SECONDS")]
        #[arg(value_parser = animation_time_parser)]
        pub on_time: Option<Time>,

        /// The off fade time (C) in fractional seconds
        #[arg(long, value_name = "SECONDS")]
        #[arg(value_parser = animation_time_parser)]
        pub off_fade_time: Option<Time>,

        /// The off time (D) in fractional seconds
        #[arg(long, value_name = "SECONDS")]
        #[arg(value_parser = animation_time_parser)]
        pub off_time: Option<Time>,
    }

    #[derive(Args)]
    pub struct FadeArgs {
        #[command(flatten)]
        pub shared: SharedArgs,

        /// A color to display during the fade; repeat to set mutliple colors.
        ///
        /// Colors are specified as CSS color strings. The alpha channel is ignored.
        #[arg(long = "color", value_name = "COLOR")]
        #[arg(value_parser = csscolorparser::parse)]
        pub colors: Vec<Color>,

        /// The fade time in fractional seconds
        #[arg(long, value_name = "SECONDS")]
        #[arg(value_parser = animation_time_parser)]
        pub fade_time: Option<Time>,

        /// The hold time in fractional seconds
        #[arg(long, value_name = "SECONDS")]
        #[arg(value_parser = animation_time_parser)]
        pub hold_time: Option<Time>,
    }

    #[derive(Args)]
    pub struct SharedArgs {
        /// The lamp that will play this animation
        #[arg(long = "lamp", value_name = "ID")]
        #[arg(value_parser = cli::lamp_id_parser)]
        pub lamp_id: u8,

        /// Save this animation in flash as the default for the lamp
        #[arg(long)]
        pub default: bool,
    }

    #[derive(Copy, Clone, Debug)]
    pub struct Time(pub u16);

    impl From<Time> for u16 {
        fn from(value: Time) -> Self {
            value.0
        }
    }

    /// Parses a fractional seconds string to u16 milliseconds.
    fn animation_time_parser(value: &str) -> Result<Time, String> {
        const MILLIS: f64 = 1000.0;
        const RANGE: RangeInclusive<f64> = 0.0..=(u16::MAX as f64) / MILLIS;

        let s: f64 = value.parse().map_err(|_| "invalid time value")?;
        if RANGE.contains(&s) {
            Ok(Time((MILLIS * s).trunc() as u16))
        } else {
            Err(format!(
                "time must be between {:.3} and {:.3} seconds",
                RANGE.start(),
                RANGE.end(),
            ))
        }
    }
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
