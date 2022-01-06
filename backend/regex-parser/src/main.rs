use regex_parser::parser::regex::RegexParser;

fn input() -> String {
    let mut buf = String::new();
    std::io::stdin().read_line(&mut buf).expect("Failed to read a line from stdin");
    // Strip the trailing newline.
    if buf.ends_with('\n') {
        buf.truncate(buf.len() - 1);
    }
    buf
}

fn main() {
    eprint!("Type a regex: ");
    let line = input();
    let parser = RegexParser::new();
    let result = parser.parse(&line);
    let _ = dbg!(result);
}
