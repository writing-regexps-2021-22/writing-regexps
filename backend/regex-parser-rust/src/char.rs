use std::convert::TryFrom;

pub trait Char: Into<char> + TryFrom<char> + Eq + PartialOrd {}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Ord, PartialOrd, Hash)]
pub struct NaiveChar(pub char);

impl From<char> for NaiveChar {
    fn from(c: char) -> NaiveChar {
        NaiveChar(c)
    }
}

impl From<NaiveChar> for char {
    fn from(nc: NaiveChar) -> char {
        nc.0
    }
}

impl Char for NaiveChar {}
