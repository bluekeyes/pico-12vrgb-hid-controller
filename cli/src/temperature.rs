use std::fmt;

#[derive(Copy, Clone, Debug)]
pub enum Degrees {
    Celsius,
    Farenheight,
}

impl fmt::Display for Degrees {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> Result<(), fmt::Error> {
        match self {
            Self::Celsius => write!(f, "°C"),
            Self::Farenheight => write!(f, "°F"),
        }
    }
}

impl Degrees {
    pub fn parse(s: &str) -> Result<Self, String> {
        let u = s.strip_prefix('°').unwrap_or(s).to_lowercase();
        match u.as_str() {
            "celsius" | "c" => Ok(Self::Celsius),
            "farenheight" | "f" => Ok(Self::Farenheight),
            _ => Err("invalid degree unit".to_string()),
        }
    }

    pub fn from_celsius(&self, c: f64) -> f64 {
        match self {
            Self::Celsius => c,
            Self::Farenheight => 9.0 * c / 5.0 + 32.0,
        }
    }
}
