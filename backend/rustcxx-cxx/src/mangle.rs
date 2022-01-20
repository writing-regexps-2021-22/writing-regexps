use std::fmt::Write;

pub trait Mangle {
    fn mangle_to(&self, out: &mut impl Write) -> std::fmt::Result;

    fn mangle(&self) -> Result<String, std::fmt::Error> {
        let mut buf = String::new();
        self.mangle_to(&mut buf)?;
        Ok(buf)
    }
}

impl<'a> Mangle for &'a str {
    fn mangle_to(&self, out: &mut impl Write) -> std::fmt::Result {
        for b in self.bytes() {
            if b.is_ascii_alphanumeric() {
                write!(out, "{}", b as char)?;
            } else {
                let hex = b"0123456789abcdef";
                let high_nibble = hex[(b >> 4) as usize] as char;
                let low_nibble = hex[(b & 0xF) as usize] as char;
                write!(out, "_{}{}", high_nibble, low_nibble)?
            }
        }
        Ok(())
    }
}
