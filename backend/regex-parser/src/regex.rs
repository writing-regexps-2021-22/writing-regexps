use crate::utils::Range;

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct Regex {
    pub root_part: RegexPart,
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub enum RegexPart {
    Empty,
    Literal(char),
    Alternatives(Vec<RegexPart>),
    Sequence(Vec<RegexPart>),
    Bracketed(Bracketed),
    ParenGroup { capture: Option<Capture>, inner: Box<RegexPart> },
    LineStart,
    LineEnd,
    Optional(Box<RegexPart>),
    ZeroOrMore { eagerness: Eagerness, inner: Box<RegexPart> },
    OneOrMore { eagerness: Eagerness, inner: Box<RegexPart> },
    Repeat { eagerness: Eagerness, n: RepeatSpec, inner: Box<RegexPart> },
}

#[derive(Debug, Clone, Eq, PartialEq)]
pub struct Bracketed {
    pub alternatives: Vec<BracketedAlternative>,
}

#[derive(Debug, Clone, Eq, PartialEq, Hash)]
pub enum BracketedAlternative {
    Single(char),
    Range(Range<char>),
}

#[derive(Debug, Clone, Eq, PartialEq, Hash)]
pub enum Capture {
    Index,
    Name { name: String, flavor: NamedCaptureFlavor },
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
pub enum NamedCaptureFlavor {
    // (?P<name>group)
    AnglesWithP,
    // (?<name>group)
    Angles,
    // (?'name'group)
    Apostrophes,
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
pub enum Eagerness {
    Greedy,
    Lazy,
    Possessive,
}

#[derive(Debug, Copy, Clone, Eq, PartialEq, Hash)]
pub enum RepeatSpec {
    Exactly(usize),
    AtLeast(usize),
    AtMost(usize),
    Range(Range<usize>),
}
