use clap::Parser;
use pico_12vrgb_ctrl::cli::Root;

fn main() {
    Root::parse().run();
}
