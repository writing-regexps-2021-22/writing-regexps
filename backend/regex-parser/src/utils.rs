use std::fmt::{self, Formatter, Display};

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
pub struct Range<T> {
    min: T,
    max: T,
}

impl<T: PartialOrd> Range<T> {
    pub fn new(min: T, max: T) -> Result<Self, RangeError<T>> {
        if min <= max {
            Ok(Self { min, max })
        } else {
            Err(RangeError { provided_min: min, provided_max: max })
        }
    }
}

impl<T> Range<T> {
    pub fn min(&self) -> &T {
        &self.min
    }

    pub fn max(&self) -> &T {
        &self.max
    }
}

#[derive(Debug, Copy, Clone, Eq, PartialEq)]
pub struct RangeError<T> {
    provided_min: T,
    provided_max: T,
}

impl<T: Display> Display for RangeError<T> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "Cannot construct a range with min ({}) greater than max ({})",
            self.provided_min,
            self.provided_max,
        )
    }
}
