use gtk::prelude::*;
use gtk::{
    Adjustment, Application, ApplicationWindow, Box as GtkBox, Label, Orientation, ScrolledWindow,
};
use reqwest::blocking::Client;
use serde::{Deserialize, Serialize};
use std::collections::HashMap;

macro_rules! server_base_address {
    () => {
        "http://127.0.0.1:6666"
    };
}

const EXPLAIN_URL: &'static str = concat!(server_base_address!(), "/explain");
const PARSE_URL: &'static str = concat!(server_base_address!(), "/parse");
const MATCH_URL: &'static str = concat!(server_base_address!(), "/match");

#[derive(Debug, Clone, Eq, PartialEq, Deserialize)]
struct ExplanationItem {
    #[serde(rename = "explanation")]
    text: String,
    depth: usize,
    bold: bool,
}

struct Styles {
    pub common: gtk::pango::AttrList,
    pub mono: gtk::pango::AttrList,
    pub monobold: gtk::pango::AttrList,
}

impl Styles {
    pub fn new() -> Self {
        let common = gtk::pango::AttrList::new();
        let mono = gtk::pango::AttrList::new();
        let monobold = gtk::pango::AttrList::new();
        let mut font = gtk::pango::FontDescription::new();
        font.set_size(12000);
        common.insert(gtk::pango::AttrFontDesc::new(&font));
        font.set_family("monospace");
        font.set_size(font.size() + 1000);
        mono.insert(gtk::pango::AttrFontDesc::new(&font));
        font.set_weight(gtk::pango::Weight::Semibold);
        monobold.insert(gtk::pango::AttrFontDesc::new(&font));
        Self {
            common,
            mono,
            monobold,
        }
    }

    pub fn select(&self, span: &Span) -> gtk::pango::AttrList {
        let base = gtk::pango::AttrList::new();
        let mut font = gtk::pango::FontDescription::new();
        font.set_size(12000);
        font.set_family("monospace");
        font.set_size(font.size() + 1000);
        base.insert(gtk::pango::AttrFontDesc::new(&font));
        let mut attr = gtk::pango::AttrColor::new_background(65535, 65535, 0);
        attr.set_start_index(span.0 as u32);
        attr.set_end_index(span.1 as u32);
        base.insert(attr);
        base
    }
}

#[derive(Debug)]
enum Command {
    ExplainRegex {
        regex: String,
        explanation: Vec<ExplanationItem>,
    },
    ShowBacktrackingMatching {
        regex: String,
        string: String,
        matching: BacktrackingMatching,
    },
}

impl Command {
    pub fn setup_header(&self, styles: &Styles) -> GtkBox {
        match self {
            Self::ExplainRegex {
                regex,
                explanation: _,
            } => {
                let hbox = GtkBox::new(Orientation::Horizontal, 0);
                let explain_label = Label::new(Some("Explain regex:"));
                let regex_label = Label::new(Some(regex));
                explain_label.set_attributes(Some(&styles.common));
                regex_label.set_attributes(Some(&styles.mono));
                hbox.pack_start(&explain_label, false, false, 20);
                hbox.pack_start(&regex_label, false, false, 20);
                hbox
            }
            Self::ShowBacktrackingMatching {
                regex,
                string,
                matching: _,
            } => {
                let hbox = GtkBox::new(Orientation::Horizontal, 0);
                let regex_text_label = Label::new(Some("Regex:"));
                let regex_label = Label::new(Some(regex));
                let string_text_label = Label::new(Some("String:"));
                let string_label = Label::new(Some(string));
                regex_text_label.set_attributes(Some(&styles.common));
                string_text_label.set_attributes(Some(&styles.common));
                regex_label.set_attributes(Some(&styles.mono));
                string_label.set_attributes(Some(&styles.mono));
                hbox.pack_start(&regex_text_label, false, false, 20);
                hbox.pack_start(&regex_label, false, false, 20);
                hbox.pack_start(&string_text_label, false, false, 20);
                hbox.pack_start(&string_label, false, false, 20);
                hbox
            }
        }
    }

    pub fn setup_data(&self, styles: &Styles) -> GtkBox {
        match self {
            Self::ExplainRegex {
                regex: _,
                explanation,
            } => {
                let vbox = GtkBox::new(Orientation::Vertical, 10);
                for item in explanation.iter() {
                    let baked_text = std::iter::repeat(' ')
                        .take(4 * item.depth)
                        .collect::<String>()
                        + &item.text;
                    let label = Label::new(Some(&baked_text));
                    label.set_attributes(Some(if item.bold {
                        &styles.monobold
                    } else {
                        &styles.mono
                    }));
                    label.set_halign(gtk::Align::Start);
                    vbox.pack_start(&label, true, true, 0);
                }
                vbox
            }
            Self::ShowBacktrackingMatching {
                regex,
                string,
                matching,
            } => {
                let wrap = GtkBox::new(Orientation::Vertical, 0);
                let grid = gtk::Grid::new();
                grid.set_row_spacing(20);

                if let Some(captures) = &matching.captures {
                    let captures_vbox = GtkBox::new(Orientation::Vertical, 10);
                    put_bold_line(&captures_vbox, "Whole capture", styles);
                    put_capture(&captures_vbox, &captures.whole, string, None, styles);
                    if !captures.by_index.is_empty() {
                        put_bold_line(&captures_vbox, "Captures by index", styles);
                        let mut data: Vec<_> = captures
                            .by_index
                            .iter()
                            .map(|(k, v)| (k.parse::<usize>().unwrap(), v.clone()))
                            .collect();
                        data.sort_by_key(|(k, _)| *k);
                        for (index, capture) in data {
                            put_capture(
                                &captures_vbox,
                                &capture,
                                string,
                                Some(&format!("{}", index)),
                                styles,
                            );
                        }
                    }
                    if !captures.by_name.is_empty() {
                        put_bold_line(&captures_vbox, "Captures by name", styles);
                        let mut data: Vec<_> = captures.by_name.iter().collect();
                        data.sort_by_key(|(k, _)| *k);
                        for (name, capture) in data {
                            put_capture(&captures_vbox, &capture, string, Some(name), styles);
                        }
                    }
                    grid.attach(&captures_vbox, 2, 1, 1, 1);
                }

                for (i, step) in matching.steps.iter().enumerate() {
                    let row = i + 2;
                    let step_num_label = Label::new(Some(&format!("{}.", i + 1)));
                    step_num_label.set_attributes(Some(&styles.common));
                    step_num_label.set_valign(gtk::Align::Start);
                    grid.attach(&step_num_label, 1, row as i32, 1, 1);
                    let vbox = put_step(step, regex, string, styles);
                    grid.attach(&vbox, 2, row as i32, 1, 1);
                }
                wrap.pack_start(&grid, false, false, 0);
                wrap
            }
        }
    }
}

fn put_step(step: &BacktrackingStep, regex: &str, string: &str, styles: &Styles) -> GtkBox {
    let vbox = GtkBox::new(Orientation::Vertical, 5);
    match step {
        BacktrackingStep::Backtrack(s) => {
            let label = Label::new(Some(&format!(
                "Go back and resume right after step {}",
                s.continue_after_step + 1
            )));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, true, true, 0);
            put_string_pos(&vbox, string, s.string_pos, styles);
        }
        BacktrackingStep::BeginGroup(s) => {
            let label = Label::new(Some("Begin group"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, true, true, 0);
            put_string_span(&vbox, regex, s.regex_span, styles);
            put_string_pos(&vbox, string, s.string_pos, styles);
        }
        BacktrackingStep::End(s) => {
            let label = Label::new(Some(if s.success {
                "Done (match found)"
            } else {
                "Done (match not found)"
            }));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, true, true, 0);
            put_string_pos(&vbox, string, s.string_pos, styles);
        }
        BacktrackingStep::EndGroup(s) => {
            let label = Label::new(Some("End group"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_pos(&vbox, string, s.string_pos, styles);
        }
        BacktrackingStep::MatchAlternatives(s) => {
            let label = Label::new(Some("Start matching alternatives"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, s.regex_span, styles);
            put_string_pos(&vbox, string, s.string_pos, styles);
        }
        BacktrackingStep::MatchPlus(s) => {
            let label = Label::new(Some("Start matching the `+` quantifier"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, s.regex_span, styles);
            put_string_pos(&vbox, string, s.string_pos, styles);
        }
        BacktrackingStep::MatchStar(s) => {
            let label = Label::new(Some("Start matching the `*` quantifier"));
            label.set_attributes(Some(&styles.common));
            vbox.pack_start(&label, false, false, 0);
            label.set_halign(gtk::Align::Start);
            put_string_span(&vbox, regex, s.regex_span, styles);
            put_string_pos(&vbox, string, s.string_pos, styles);
        }
        BacktrackingStep::MatchOptional(s) => {
            let label = Label::new(Some("Start matching the `?` quantifier"));
            label.set_attributes(Some(&styles.common));
            vbox.pack_start(&label, false, false, 0);
            label.set_halign(gtk::Align::Start);
            put_string_span(&vbox, regex, s.regex_span, styles);
            put_string_pos(&vbox, string, s.string_pos, styles);
        }
        BacktrackingStep::FinishAlternatives(FinishAlternativesStep::Success {
            regex_span,
            string_span,
            alternative_chosen,
        }) => {
            let label = Label::new(Some(&format!(
                "Alternative #{} has been matched",
                alternative_chosen + 1
            )));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_span(&vbox, string, *string_span, styles);
        }
        BacktrackingStep::FinishAlternatives(FinishAlternativesStep::Failure {
            regex_span,
            string_pos,
            failure_reason,
        }) => {
            let label = Label::new(Some(&"Failed to choose an alternative"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_pos(&vbox, string, *string_pos, styles);
            put_failure_reason(&vbox, *failure_reason, styles);
        }
        BacktrackingStep::FinishStar(FinishQuantifierStep::Success {
            regex_span,
            string_span,
            num_repetitions,
        }) => {
            let label = Label::new(Some(&format!(
                "Finished matching the `*` quantifier (# repetitions: {})",
                num_repetitions
            )));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_span(&vbox, string, *string_span, styles);
        }
        BacktrackingStep::FinishStar(FinishQuantifierStep::Failure {
            regex_span,
            string_pos,
            failure_reason,
        }) => {
            let label = Label::new(Some("Failed to matching the `*` quantifier"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_pos(&vbox, string, *string_pos, styles);
            put_failure_reason(&vbox, *failure_reason, styles);
        }
        BacktrackingStep::FinishPlus(FinishQuantifierStep::Success {
            regex_span,
            string_span,
            num_repetitions,
        }) => {
            let label = Label::new(Some(&format!(
                "Finished matching the `+` quantifier (# repetitions: {})",
                num_repetitions
            )));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_span(&vbox, string, *string_span, styles);
        }
        BacktrackingStep::FinishPlus(FinishQuantifierStep::Failure {
            regex_span,
            string_pos,
            failure_reason,
        }) => {
            let label = Label::new(Some("Failed to matching the `+` quantifier"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_pos(&vbox, string, *string_pos, styles);
            put_failure_reason(&vbox, *failure_reason, styles);
        }
        BacktrackingStep::FinishOptional(FinishQuantifierStep::Success {
            regex_span,
            string_span,
            num_repetitions,
        }) => {
            let label = Label::new(Some(&format!(
                "Finished matching the `?` quantifier (# repetitions: {})",
                num_repetitions
            )));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_span(&vbox, string, *string_span, styles);
        }
        BacktrackingStep::FinishOptional(FinishQuantifierStep::Failure {
            regex_span,
            string_pos,
            failure_reason,
        }) => {
            let label = Label::new(Some("Failed to matching the `?` quantifier"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_pos(&vbox, string, *string_pos, styles);
            put_failure_reason(&vbox, *failure_reason, styles);
        }
        BacktrackingStep::MatchCharClass(MatchCharClassStep::Success {
            regex_span,
            string_span,
        }) => {
            let label = Label::new(Some("Match the character class"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_span(&vbox, string, *string_span, styles);
        }
        BacktrackingStep::MatchCharClass(MatchCharClassStep::Failure {
            regex_span,
            string_pos,
            failure_reason,
        }) => {
            let label = Label::new(Some("Failed to match the character class"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_pos(&vbox, string, *string_pos, styles);
            put_failure_reason(&vbox, *failure_reason, styles);
        }
        BacktrackingStep::MatchLiteral(MatchLiteralStep::Success {
            regex_span,
            literal,
            string_span,
        }) => {
            let label = Label::new(Some(&format!("Match the literal {:?}", literal)));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_span(&vbox, string, *string_span, styles);
        }
        BacktrackingStep::MatchLiteral(MatchLiteralStep::Failure {
            regex_span,
            literal,
            string_pos,
            failure_reason,
        }) => {
            let label = Label::new(Some(&format!("Failed to match the literal {:?}", literal)));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_pos(&vbox, string, *string_pos, styles);
            put_failure_reason(&vbox, *failure_reason, styles);
        }
        BacktrackingStep::MatchWildcard(MatchWildcardStep::Success {
            regex_span,
            string_span,
        }) => {
            let label = Label::new(Some("Match the wildcard"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_span(&vbox, string, *string_span, styles);
        }
        BacktrackingStep::MatchWildcard(MatchWildcardStep::Failure {
            regex_span,
            string_pos,
            failure_reason,
        }) => {
            let label = Label::new(Some("Failed to match the wildcard"));
            label.set_attributes(Some(&styles.common));
            label.set_halign(gtk::Align::Start);
            vbox.pack_start(&label, false, false, 0);
            put_string_span(&vbox, regex, *regex_span, styles);
            put_string_pos(&vbox, string, *string_pos, styles);
            put_failure_reason(&vbox, *failure_reason, styles);
        }
    }
    vbox
}

fn put_failure_reason(vbox: &GtkBox, reason: FailureReason, styles: &Styles) {
    let text = match reason.code {
        FailureReasonCode::EndOfInput => "the input string has ended prematurely",
        FailureReasonCode::ExcludedChar => {
            "the character from the string was not in the allowed set"
        }
        FailureReasonCode::OtherChar => {
            "the character from the string did not coincide with the pattern"
        }
        FailureReasonCode::OptionsExhausted => "no more options were left",
    };
    let label = Label::new(Some(&format!("Matching failed because {}", text)));
    label.set_attributes(Some(&styles.common));
    label.set_halign(gtk::Align::Start);
    vbox.pack_start(&label, false, false, 0);
}

fn put_string_pos(vbox: &GtkBox, string: &str, pos: usize, styles: &Styles) {
    put_string_span(vbox, string, Span(pos, pos + 1), styles);
}

fn put_string_span(vbox: &GtkBox, string: &str, span: Span, styles: &Styles) {
    let label = Label::new(Some(string));
    label.set_attributes(Some(&styles.select(&span)));
    label.set_halign(gtk::Align::Start);
    vbox.pack_start(&label, false, false, 0);
}

fn put_bold_line(vbox: &GtkBox, line: &str, styles: &Styles) {
    let label = Label::new(Some(line));
    label.set_attributes(Some(&styles.monobold));
    label.set_halign(gtk::Align::Start);
    vbox.pack_start(&label, false, false, 0);
}

fn put_capture(
    vbox: &GtkBox,
    capture: &Capture,
    string: &str,
    info: Option<&str>,
    styles: &Styles,
) {
    let hbox = GtkBox::new(Orientation::Horizontal, 10);
    let text_label = Label::new(Some(&format!(
        "{}[index {} length {}]:",
        match info {
            Some(i) => format!("{} ", i),
            None => String::from(""),
        },
        capture.string_span.index(),
        capture.string_span.length()
    )));
    text_label.set_attributes(Some(&styles.mono));
    let string_label = Label::new(Some(string));
    string_label.set_attributes(Some(&styles.select(&capture.string_span)));
    hbox.pack_start(&text_label, false, false, 0);
    hbox.pack_start(&string_label, false, false, 0);
    vbox.pack_start(&hbox, false, false, 0);
}

struct App {
    command: Command,
}

impl App {
    pub fn new(command: Command) -> Self {
        Self { command }
    }

    pub fn run(self) {
        let styles = Styles::new();
        let application = Application::new(None, gtk::gio::ApplicationFlags::NON_UNIQUE);
        application.connect_command_line(|_, _| 0);
        application.connect_activate(move |app| {
            let main_window = ApplicationWindow::builder()
                .application(app)
                .default_width(800)
                .default_height(600)
                .title("Writing Regexps With Pleasure")
                .build();

            let main_vbox = GtkBox::new(Orientation::Vertical, 50);
            main_vbox.set_margin(20);
            main_vbox.pack_start(&self.command.setup_header(&styles), false, false, 0);
            main_vbox.pack_start(&self.command.setup_data(&styles), false, false, 0);
            let no_adjustment: Option<&Adjustment> = None;
            let scrolled = ScrolledWindow::new(no_adjustment, no_adjustment);
            scrolled.add(&main_vbox);
            main_window.add(&scrolled);
            main_window.show_all();
        });
        application.run_with_args::<&'static str>(&[]);
    }
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "snake_case")]
enum Response<Data> {
    Error(ServiceError),
    Data(Data),
}

#[derive(Debug, Deserialize)]
struct ServiceError {
    pub code: ServiceErrorCode,
}

impl std::fmt::Display for ServiceError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let text = match self.code {
            ServiceErrorCode::InternalError => "Internal error",
            ServiceErrorCode::InvalidRequestJson => "Invalid request JSON",
            ServiceErrorCode::InvalidRequestJsonStructure => "Invalid request JSON structure",
            ServiceErrorCode::InvalidUtf8 => "Invalid UTF-8",
            ServiceErrorCode::NotImplemented => "Not implemented",
        };
        write!(f, "{}", text)
    }
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "snake_case")]
enum ServiceErrorCode {
    InternalError,
    InvalidRequestJson,
    InvalidRequestJsonStructure,
    InvalidUtf8,
    NotImplemented,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "snake_case")]
enum ExplanationResult {
    Explanation(Vec<ExplanationItem>),
    ParseError(ParseError),
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "snake_case")]
enum ParseResult {
    ParseTree {},
    ParseError(ParseError),
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "snake_case")]
enum MatchResult {
    MatchResults(Vec<Matching>),
    ParseError(ParseError),
}

#[derive(Debug, Deserialize)]
struct Matching(pub BacktrackingMatching);

#[derive(Debug, Deserialize)]
struct BacktrackingMatching {
    pub matched: bool,
    pub captures: Option<Captures>,
    pub steps: Vec<BacktrackingStep>,
}

#[derive(Debug, Deserialize, Clone)]
struct Captures {
    pub whole: Capture,
    pub by_index: HashMap<String, Capture>,
    pub by_name: HashMap<String, Capture>,
}

#[derive(Debug, Deserialize, Clone)]
struct Capture {
    pub string_span: Span,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "snake_case")]
#[serde(tag = "type")]
enum BacktrackingStep {
    MatchLiteral(MatchLiteralStep),
    MatchWildcard(MatchWildcardStep),
    MatchCharClass(MatchCharClassStep),
    MatchStar(MatchQuantifierStep),
    MatchPlus(MatchQuantifierStep),
    MatchOptional(MatchQuantifierStep),
    FinishStar(FinishQuantifierStep),
    FinishPlus(FinishQuantifierStep),
    FinishOptional(FinishQuantifierStep),
    BeginGroup(BeginGroupStep),
    EndGroup(EndGroupStep),
    MatchAlternatives(MatchAlternativesStep),
    FinishAlternatives(FinishAlternativesStep),
    Backtrack(BacktrackStep),
    End(EndStep),
}

#[derive(Debug, Copy, Clone, Deserialize)]
struct Span(pub usize, pub usize);

impl Span {
    pub fn index(&self) -> usize {
        self.0
    }

    pub fn length(&self) -> usize {
        self.1 - self.0
    }
}

#[derive(Debug, Copy, Clone, Deserialize)]
#[serde(rename_all = "snake_case")]
enum FailureReasonCode {
    OtherChar,
    EndOfInput,
    ExcludedChar,
    OptionsExhausted,
}

#[derive(Debug, Copy, Clone, Deserialize)]
struct FailureReason {
    pub code: FailureReasonCode,
}

#[derive(Debug, Deserialize)]
#[serde(untagged)]
enum MatchLiteralStep {
    Success {
        regex_span: Span,
        literal: char,
        string_span: Span,
    },
    Failure {
        regex_span: Span,
        literal: char,
        string_pos: usize,
        failure_reason: FailureReason,
    },
}

#[derive(Debug, Deserialize)]
#[serde(untagged)]
enum MatchWildcardStep {
    Success {
        regex_span: Span,
        string_span: Span,
    },
    Failure {
        regex_span: Span,
        string_pos: usize,
        failure_reason: FailureReason,
    },
}

#[derive(Debug, Deserialize)]
#[serde(untagged)]
enum MatchCharClassStep {
    Success {
        regex_span: Span,
        string_span: Span,
    },
    Failure {
        regex_span: Span,
        string_pos: usize,
        failure_reason: FailureReason,
    },
}

#[derive(Debug, Deserialize)]
#[serde(untagged)]
enum FinishQuantifierStep {
    Success {
        regex_span: Span,
        string_span: Span,
        num_repetitions: usize,
    },
    Failure {
        regex_span: Span,
        string_pos: usize,
        failure_reason: FailureReason,
    },
}

#[derive(Debug, Deserialize)]
#[serde(untagged)]
enum FinishAlternativesStep {
    Success {
        regex_span: Span,
        string_span: Span,
        alternative_chosen: usize,
    },
    Failure {
        regex_span: Span,
        string_pos: usize,
        failure_reason: FailureReason,
    },
}

#[derive(Debug, Deserialize)]
struct MatchQuantifierStep {
    pub regex_span: Span,
    pub string_pos: usize,
}

#[derive(Debug, Deserialize)]
struct BeginGroupStep {
    pub regex_span: Span,
    pub string_pos: usize,
}

#[derive(Debug, Deserialize)]
struct EndGroupStep {
    pub string_pos: usize,
}

#[derive(Debug, Deserialize)]
struct MatchAlternativesStep {
    pub regex_span: Span,
    pub string_pos: usize,
}

#[derive(Debug, Deserialize)]
struct BacktrackStep {
    pub string_pos: usize,
    pub continue_after_step: usize,
}

#[derive(Debug, Deserialize)]
struct EndStep {
    pub string_pos: usize,
    pub success: bool,
}

#[derive(Debug, Deserialize)]
struct ParseErrorHint {
    pub hint: String,
}

#[derive(Debug, Deserialize)]
struct ParseErrorData {
    pub hint: ParseErrorHint,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "snake_case")]
#[serde(tag = "code")]
enum ParseError {
    ExpectedEnd { data: ParseErrorData },
    UnexpectedChar { data: ParseErrorData },
    UnexpectedEnd { data: ParseErrorData },
    InvalidRange { data: ParseErrorData },
}

impl std::fmt::Display for ParseError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Self::ExpectedEnd { data }
            | Self::UnexpectedChar { data }
            | Self::UnexpectedEnd { data }
            | Self::InvalidRange { data } => write!(f, "{}", data.hint.hint),
        }
    }
}

fn post<O: for<'a> Deserialize<'a>>(url: &str, body: &impl Serialize) -> O {
    let client = Client::new();

    let request_json = serde_json::to_string(body).unwrap();
    let http_response = client.post(url).body(request_json).send().unwrap();
    let http_response_json = http_response.text().unwrap();
    serde_json::from_str(&http_response_json).unwrap()
}

#[derive(Debug, Serialize)]
struct SimpleRequest<'a> {
    pub regex: &'a str,
}

fn do_explain_regex(regex: &str) -> Option<Command> {
    let response = post(EXPLAIN_URL, &SimpleRequest { regex });
    match response {
        Response::Error(e) => {
            println!("Service error: {}", e);
            None
        }
        Response::Data(ExplanationResult::ParseError(e)) => {
            println!("Parse error: {}", e);
            None
        }
        Response::Data(ExplanationResult::Explanation(explanation)) => {
            println!("OK");
            Some(Command::ExplainRegex {
                regex: regex.to_owned(),
                explanation,
            })
        }
    }
}

fn do_parse_regex(regex: &str) -> Option<Command> {
    let response = post(PARSE_URL, &SimpleRequest { regex });
    match response {
        Response::Error(e) => {
            println!("Service error: {}", e);
            None
        }
        Response::Data(ParseResult::ParseError(e)) => {
            println!("Parse error: {}", e);
            None
        }
        Response::Data(ParseResult::ParseTree {}) => {
            println!("OK");
            None
        }
    }
}

#[derive(Debug, Serialize)]
struct MatchRequest<'a, 'b, 'c> {
    pub regex: &'a str,
    pub strings: &'b [MatchString<'c>],
}

#[derive(Debug, Serialize)]
struct MatchString<'a> {
    pub string: &'a str,
    pub fragment: MatchStringFragment,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "snake_case")]
enum MatchStringFragment {
    Whole,
}

fn do_match_regex(regex: &str, string: &str) -> Option<Command> {
    let match_string = MatchString {
        string,
        fragment: MatchStringFragment::Whole,
    };
    let strings = std::slice::from_ref(&match_string);
    let response = post(MATCH_URL, &MatchRequest { regex, strings });
    match response {
        Response::Error(e) => {
            println!("Service error: {}", e);
            None
        }
        Response::Data(MatchResult::ParseError(e)) => {
            println!("Parse error: {}", e);
            None
        }
        Response::Data(MatchResult::MatchResults(matchings)) => {
            println!("OK");
            let matching = matchings.into_iter().next().unwrap();
            Some(Command::ShowBacktrackingMatching {
                regex: regex.to_owned(),
                string: string.to_owned(),
                matching: matching.0,
            })
        }
    }
}

fn main() {
    let args: Vec<_> = std::env::args().skip(1).collect();
    let args_str: Vec<_> = args.iter().map(String::as_str).collect();
    let command = match args_str[..] {
        ["explain", regex] => do_explain_regex(regex),
        ["parse", regex] => do_parse_regex(regex),
        ["match", regex, string] => do_match_regex(regex, string),
        _ => panic!("Invalid usage"),
    };
    if let Some(c) = command {
        App::new(c).run()
    }
}
