pub struct Separator {
    sep: &'static str,
    first: bool,
}

impl Separator {
    pub fn new(sep: &'static str) -> Self {
        Self { sep, first: true }
    }

    pub fn next(&mut self) -> &'static str {
        if self.first {
            self.first = false;
            ""
        } else {
            self.sep
        }
    }

    pub fn as_fn_mut<'a>(&'a mut self) -> impl (FnMut() -> &'static str) + 'a {
        || self.next()
    }
}
