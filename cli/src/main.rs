use std::process;

use clap::Parser;
use pico_12vrgb_ctrl::cli::Root;

fn main() {
    if let Err(err) = Root::parse().run() {
        eprintln!("Error: {}", err.to_string());
        process::exit(1);
    }
}
