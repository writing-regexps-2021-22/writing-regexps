use crate::parser::regex::RegexParser;
use crate::regex::{Capture, NamedCaptureFlavor, RegexPart};

macro_rules! lit {
    ($s:expr) => {
        RegexPart::Sequence($s.chars().map(RegexPart::Literal).collect())
    };
    (char $c:expr) => {
        RegexPart::Literal($c)
    };
}

#[test]
fn test_basics() {
    let parser = RegexParser::new();
    assert_eq!(parser.parse("foo"), Ok(lit!("foo")));
    assert_eq!(parser.parse("x"), Ok(lit!(char 'x')));
    assert_eq!(parser.parse(""), Ok(RegexPart::Empty));
    assert_eq!(parser.parse("тест юникода"), Ok(lit!("тест юникода")));
    assert_eq!(parser.parse("\t  whitespace   "), Ok(lit!("\t  whitespace   ")));
    //assert_eq!(parser.parse("\r\n   \t\n"), Ok(lit!("\r\n   \t\n"))); // Fails
}

#[test]
fn test_alternatives() {
    let parser = RegexParser::new();
    assert_eq!(
        parser.parse("foo|bar"),
        Ok(RegexPart::Alternatives(vec![lit!("foo"), lit!("bar")]))
    );
    assert_eq!(
        parser.parse("foo|x"),
        Ok(RegexPart::Alternatives(vec![lit!("foo"), lit!(char 'x')]))
    );
    assert_eq!(
        parser.parse("a|b|c|d|e"),
        Ok(RegexPart::Alternatives(vec![
            lit!(char 'a'),
            lit!(char 'b'),
            lit!(char 'c'),
            lit!(char 'd'),
            lit!(char 'e'),
        ]))
    );
}

#[test]
fn test_groups() {
    let parser = RegexParser::new();
    assert_eq!(
        parser.parse("(aaa)"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Index),
            inner: Box::new(lit!("aaa")),
        })
    );
    assert_eq!(
        parser.parse("(a)"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Index),
            inner: Box::new(lit!(char 'a')),
        })
    );
    assert_eq!(
        parser.parse("(?:foobar)"),
        Ok(RegexPart::ParenGroup {
            capture: None,
            inner: Box::new(lit!("foobar")),
        })
    );
    assert_eq!(
        parser.parse("(?<x>foobar)"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Name {
                name: String::from("x"),
                flavor: NamedCaptureFlavor::Angles,
            }),
            inner: Box::new(lit!("foobar")),
        })
    );
    assert_eq!(
        parser.parse("(?<quux>12345)"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Name {
                name: String::from("quux"),
                flavor: NamedCaptureFlavor::Angles,
            }),
            inner: Box::new(lit!("12345")),
        })
    );
    assert_eq!(
        parser.parse("(?'abc123'xyz)"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Name {
                name: String::from("abc123"),
                flavor: NamedCaptureFlavor::Apostrophes,
            }),
            inner: Box::new(lit!("xyz")),
        })
    );
    assert_eq!(
        parser.parse("(?P<name>group)"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Name {
                name: String::from("name"),
                flavor: NamedCaptureFlavor::AnglesWithP,
            }),
            inner: Box::new(lit!("group")),
        })
    );
    assert_eq!(
        parser.parse("(?P<тест>юникода)"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Name {
                name: String::from("тест"),
                flavor: NamedCaptureFlavor::AnglesWithP,
            }),
            inner: Box::new(lit!("юникода")),
        })
    );
    assert_eq!(
        parser.parse("(?<a>)"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Name {
                name: String::from("a"),
                flavor: NamedCaptureFlavor::Angles,
            }),
            inner: Box::new(RegexPart::Empty),
        })
    );
    assert_eq!(
        parser.parse("(?'bb')"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Name {
                name: String::from("bb"),
                flavor: NamedCaptureFlavor::Apostrophes,
            }),
            inner: Box::new(RegexPart::Empty),
        })
    );
    assert_eq!(
        parser.parse("(?P<ccc>)"),
        Ok(RegexPart::ParenGroup {
            capture: Some(Capture::Name {
                name: String::from("ccc"),
                flavor: NamedCaptureFlavor::AnglesWithP,
            }),
            inner: Box::new(RegexPart::Empty),
        })
    );
}
