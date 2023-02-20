use clap::{Args, Parser, Subcommand};
use csscolorparser::{self, Color};
use pico_12vrgb_ctrl::temperature;

#[derive(Parser)]
#[command(author, version, about, long_about = None)]
struct Cli {
    #[command(subcommand)]
    command: Commands,
}

#[derive(Subcommand)]
enum Commands {
    /// Control lights directly
    LampArray {
        #[command(subcommand)]
        command: LampArrayCommands,
    },

    /// Manage built-in animations
    #[command(subcommand_value_name("TYPE"))]
    #[command(subcommand_help_heading("Animation Types"))]
    #[command(disable_help_subcommand(true))]
    SetAnimation {
        #[command(subcommand)]
        animation_type: AnimationTypes,
    },

    /// Reset the controller hardware
    Reset(ResetArgs),

    /// Read the internal temperature sensor
    GetTemperature(GetTemperatureArgs),
}

#[derive(Subcommand)]
enum LampArrayCommands {
    /// Set lamp array controls
    SetControl {
        /// Enable or disable autonomous mode
        #[arg(long)]
        autonomous: Option<bool>,
    },

    /// Set lamp colors
    Update {
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
    },

    /// Set a range of lamps to a color
    UpdateRange {
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
    },
}

#[derive(Subcommand)]
enum AnimationTypes {
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
    Breathe(BreathAnimationArgs),

    /// Crossfade between multiple colors
    Fade(FadeAnimationArgs),
}

#[derive(Args)]
struct BreathAnimationArgs {
    #[command(flatten)]
    shared: SharedAnimationArgs,

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
struct FadeAnimationArgs {
    #[command(flatten)]
    shared: SharedAnimationArgs,

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
struct SharedAnimationArgs {
    /// The lamp that will play this animation
    #[arg(long = "lamp", value_name = "ID")]
    lamp_id: u8,

    /// Save this animation in flash as the default for the lamp
    #[arg(long)]
    default: bool,
}

#[derive(Args)]
struct ResetArgs {
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
struct GetTemperatureArgs {
    /// The units to use when printing the temperature
    #[arg(short, long, default_value_t = temperature::Degrees::Celsius)]
    #[arg(value_parser = temperature::Degrees::parse)]
    units: temperature::Degrees,

    /// How often to print the temperature, in seconds. If unset, print the current temperature
    /// and exit.
    interval: Option<u32>,
}

fn main() {
    let cli = Cli::parse();

    match &cli.command {
        Commands::Reset(args) => println!(
            "Resetting! bootsel={0} clear_flash={1}",
            args.bootsel, args.clear_flash
        ),
        _ => println!("TODO(bkeyes): implement this command"),
    }
}
