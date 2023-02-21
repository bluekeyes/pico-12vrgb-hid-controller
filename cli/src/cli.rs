use clap::{Args, Parser, Subcommand};

use crate::device::{self, Device, Report, ResetReport};
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

        match &self.command {
            Commands::LampArray { command: _ } => todo!(),

            Commands::SetAnimation { animation_type: _ } => todo!(),

            Commands::Reset(args) => Device::open(vid, pid)
                .and_then(|d| {
                    d.send_report(Report::Reset(ResetReport {
                        bootsel: args.bootsel,
                        clear_flash: args.clear_flash,
                    }))
                })
                .map_err(From::from),

            Commands::GetTemperature(_) => todo!(),
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
        animation_type: animation::Types,
    },

    /// Reset the controller hardware
    Reset(ResetArgs),

    /// Read the internal temperature sensor
    GetTemperature(GetTemperatureArgs),
}

mod lamparray {
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
        autonomous: Option<bool>,
    }

    #[derive(Args)]
    pub struct UpdateArgs {
        /// The lamp ID to update; repeat to update more than one lamp. The number of lamps must
        /// match the number of colors.
        #[arg(long = "lamp", value_name = "ID")]
        lamp_ids: Vec<u8>,

        /// The new color of the lamp; repeat to update more than one lamp. The number of colors
        /// must match the number of lamps.
        ///
        /// Colors are specified as CSS color strings. The alpha channel is ignored. To turn
        /// off a lamp, set the color to black (#000000).
        #[arg(long = "color", value_name = "COLOR")]
        #[arg(value_parser = csscolorparser::parse)]
        colors: Vec<Color>,
    }

    #[derive(Args)]
    pub struct UpdateRangeArgs {
        /// The first lamp in the update range. If unset, use the first lamp on the device.
        #[arg(long = "start", value_name = "ID")]
        start_lamp_id: Option<u8>,

        /// The last lamp in the update range. If unset, use the last lamp on the device.
        #[arg(long = "end", value_name = "ID")]
        end_lamp_id: Option<u8>,

        /// The new color of the lamps. If unset, turn off the lamps in the range.
        ///
        /// Colors are specified as CSS color strings. The alpha channel is ignored. To turn
        /// off a lamp, set the color to black (#000000).
        #[arg(long)]
        #[arg(value_parser = csscolorparser::parse)]
        color: Option<Color>,
    }
}

pub mod animation {
    use clap::{Args, Subcommand};
    use csscolorparser::{self, Color};

    #[derive(Subcommand)]
    pub enum Types {
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
        shared: SharedArgs,

        /// The on color for the animation.
        ///
        /// Colors are specified as CSS color strings. The alpha channel is ignored.
        #[arg(long)]
        #[arg(value_parser = csscolorparser::parse)]
        on_color: Color,

        /// The off color for the animation. If unset, use the same color as in the "on" state.
        ///
        /// Colors are specified as CSS color strings. The alpha channel is ignored.
        #[arg(long)]
        #[arg(value_parser = csscolorparser::parse)]
        off_color: Option<Color>,

        /// The on fade time (A) in fractional seconds
        #[arg(long)]
        #[arg(value_name = "SECONDS")]
        on_fade_time: Option<f64>,

        /// The on time (B) in fractional seconds
        #[arg(long)]
        #[arg(value_name = "SECONDS")]
        on_time: Option<f64>,

        /// The off fade time (C) in fractional seconds
        #[arg(long)]
        #[arg(value_name = "SECONDS")]
        off_fade_time: Option<f64>,

        /// The off time (D) in fractional seconds
        #[arg(long)]
        #[arg(value_name = "SECONDS")]
        off_time: Option<f64>,
    }

    #[derive(Args)]
    pub struct FadeArgs {
        #[command(flatten)]
        shared: SharedArgs,

        /// A color to display during the fade; repeat to set mutliple colors.
        ///
        /// Colors are specified as CSS color strings. The alpha channel is ignored.
        #[arg(long = "color", value_name = "COLOR")]
        #[arg(value_parser = csscolorparser::parse)]
        colors: Vec<Color>,

        /// The fade time in fractional seconds
        #[arg(long)]
        #[arg(value_name = "SECONDS")]
        fade_time: Option<f64>,

        /// The hold time in fractional seconds
        #[arg(long)]
        #[arg(value_name = "SECONDS")]
        hold_time: Option<f64>,
    }

    #[derive(Args)]
    pub struct SharedArgs {
        /// The lamp that will play this animation
        #[arg(long = "lamp", value_name = "ID")]
        lamp_id: u8,

        /// Save this animation in flash as the default for the lamp
        #[arg(long)]
        default: bool,
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
