use clap::{Args, CommandFactory, Subcommand};
use csscolorparser::{self, Color};
use std::ops::RangeInclusive;

use crate::cli;
use crate::device::{self, Device, Report};

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

impl Animation {
    pub fn run(&self, dev: &Device) -> Result<(), Box<dyn std::error::Error>> {
        match self {
            Self::Breathe(args) => {
                const DEFAULT_ON_TIME: Time = Time(500);
                const DEFAULT_FADE_TIME: Time = Time(1000);
                const DEFAULT_OFF_TIME: Time = Time(1000);

                let on_color = &args.on_color;
                let off_color = args.off_color.as_ref().unwrap_or(on_color);

                dev.send_report(Report::SetAnimation(
                    args.shared.mode(),
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

            Self::Fade(args) => {
                const MAX_COLORS: usize = device::FadeAnimationData::MAX_COLORS;
                const DEFAULT_FADE_TIME: Time = Time(2000);
                const DEFAULT_HOLD_TIME: Time = Time(1000);

                let color_count = args.colors.len();
                if color_count > MAX_COLORS {
                    let mut err = cli::Root::command();
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

                dev.send_report(Report::SetAnimation(
                    args.shared.mode(),
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
        }
    }
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

impl SharedArgs {
    fn mode(&self) -> device::SetAnimationMode {
        if self.default {
            device::SetAnimationMode::Default
        } else {
            device::SetAnimationMode::Current
        }
    }
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
