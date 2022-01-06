use crate::parser::regex::RegexParser;
use crate::regex::RegexPart;

#[test]
fn test_regex_parser() {
    let parser = RegexParser::new();

    assert_eq!(
        parser.parse("foo"),
        Ok(RegexPart::Sequence(vec![
            RegexPart::Literal('f'),
            RegexPart::Literal('o'),
            RegexPart::Literal('o')
        ])),
    );
    assert_eq!(
        parser.parse("x"),
        Ok(RegexPart::Literal('x')),
    );
    assert_eq!(
        parser.parse(""),
        Ok(RegexPart::Empty),
    );
}
