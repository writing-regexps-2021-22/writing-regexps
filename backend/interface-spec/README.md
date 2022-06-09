# Communication Interface Specification v 0.2.1

## Common Definitions And Convention
1. **Backend** is the program that implements processing of regular expressions and provides
   a software interface to execute these operations on given data and obtain the result of their
   execution.
2. **Frontend** is the program that instructs the backend to
   execute operations with regular expressions, as requested, directly or indirectly, by the
   user, and presents their results, possibly with some additional post-processing, to the user.
3. **Communication interface** is the interface to the operations provided by the backend and
   used by the frontend.
4. **Service error** is an error that prevents a request
   from the frontend from being handled by the backend.  This category of errors includes
   (but is not limited to) I/O errors, errors related to the syntax or format of ths request
   or hitting a timeout, but does not include an unsuccessful result of an operation executed.
5. **Error code** is a unique string that describes a kind of error.
6. Terms **JSON object**, **JSON array**, **JSON number**, **JSON string** are defined as
   the corresponding JSON datatypes. The term **JSON value** corresponds to a value
   of an arbitrary type (explicitly including `null`).
7. Unless explicitly specified otherwise, the described values cannot be null.
8. **Bold** text is used to introduce new terms. *Italic* text is used to refer to already defined terms.

## Communication Interface
The communication interface is defined in this section.  The *backend* listens on the TCP network
socket bound to the IPv4 address `127.0.0.1` to the port `6666` and accepts incoming connections
from the *frontend*. After the TCP connection has been established, the communication between
the *frontend* and the *backend* proceeds over HTTP/1.1.  The *frontend* makes HTTP requests to the
*backend* and receives HTTP responses from it.

The set of allowed request paths is limited, and each path represents an operation with regular
expressions.  If the *frontend* requests a path not defined in this specification, the *backend*
responds with the 404 Not Found HTTP status code and arbitrary, possibly empty response body.
If the *frontend* requests a path that is defined in this specification but uses an HTTP method
that is not defined for this path, the *backend* responds with the 405 Method Not Allowed HTTP
status code and arbitrary, possibly empty response body, for any HTTP method except `HEAD`.
The behavior of the `HEAD` method is defined as in the
[HTTP/1.1 specification](https://datatracker.ietf.org/doc/rfc2616/).

When requests have a body, it must be a valid JSON document.  This document is referred to as
**the request payload**.  The structure of the request payload is defined separately for each
request path.

The response body must be a valid JSON document. The root document item must be a *JSON object*
with either one or both the following fields:

1. `data` — A *JSON object* of the format specific to the request path. Represents
   the operation-specific data returned as the result of its execution. Must be
   present if `error` is not present, but may be present even together with `error`.
   This object is referred to as **the response payload**.
2. `error` — A *JSON object* representing an error (format defined below). Describes the *service error*
   that prevented the execution of the requested operation. Must be present if and only
   if a *service error* has occurred. This field is referred to as **the response service error object**.

If present, the `error` field is an **error** object. The latter is defined as a *JSON object*
with the following fields:

1. `code` — A *JSON string* that represents the *error code*. Must always be present.
2. `data` — A *JSON value* of the error-specific format. The precise format is defined together with
   the definition of the corresponding *error code*. If no definition of the `data` field is given for an
   *error code*, this field must be absent if such error is returned.

The list of defined request paths is as follows:

1. `/parse`

### Request Paths
#### `/parse`
Parse a regular expression into its syntax tree.

Allowed HTTP methods: `POST`.

*Request payload* fields:

1. `regex` — A JSON string that represents the regular expression to parse. Must always be present.

*Response payload* is a *parse result* object representing the result of the parse operation.
This and other object types are defined below.

1. **Parse result** is a *JSON object*. Two fields are possible, and exactly one must be present:
    1. `parse_tree` — a *spanned tree node* object corresponding to the root parse tree node,
       if the regular expression was successfully parsed.
    2. `parse_error` — a **parse error** object describing the parse error if it has occurred.
       Each **parse error** object is an *error* object with the following possible *error codes*:
        1. "`expected_end`" — if a regular expression was expected to end at a certain position but did not.
           It is unspecified when exactly this error will be raised, and it may be subject to change.
           Error `data` is a *JSON object* with the following fields:
            1. `char_got` — a *JSON string* encoding exactly 1 character (Unicode codepoint). This is the character
               that was encountered instead of the end of string (that is, `char = regex[position]`).
            2. `position` — a *JSON number* representing the 0-based index of the character
               right after the expected end of the regular expression.
        2. "`unexpected_char`" — if a certain character in the regular expression is not allowable
           at its position.
           Error `data` is a *JSON object* with the following fields:
            1. `char_got` — a *JSON string* encoding exactly 1 character (Unicode codepoint). This is the
               first character that was encountered but not allowable.
            2. `position` — a *JSON number* representing the 0-based index of the first character
               that was not allowable.
            3. `expected` — a *JSON string* giving a hint on what kinds of characters were expected/allowable
               at this position. This string does not have a defined format and should only be used
               to help the developers or users to debug their parse error. This may be subject to change
               in further revisions of the specification.
        3. "`unexpected_end`" — if a regular expression ended abruptly, at a position where it was not
           expected to.
           Error `data` is a *JSON object* with the following fields:
            1. `position` — a *JSON number* representing the 0-based index of the character that would
               be located right after the end. Effectively, this is the length of the regular expression
               string.
            2. `expected` — a *JSON string* giving a hint on what kinds of characters were expected
               at this position. This string does not have a defined format and should only be used
               to help the developers or users to debug their parse error. This may be subject to change
               in further revisions of the specification.
        4. "`invalid_range`" — if a character range is invalid (wrong direction, e.g. `z-a`).
           Error `data` is a *JSON object* with the following fields:
            1. `span` — the *span* of the range in question.
            2. `first` — a *JSON string* consisting of exactly one character.
               This character is the first character in the range, as written in the regex.
               E.g. in the range `z-a` it is "`z`".
            3. `last` — a *JSON string* consisting of exactly one character.
               This character is the last character in the range, as written in the regex.
               E.g. in the range `z-a` it is "`a`".
2. **Spanned tree node** is a *JSON object* with the following fields:
    1. `span` — a *span*. Determines the starting and ending positions of the current
       parse tree node in the regular expression string.
    2. All fields from a *tree node* object.
3. **Tree node** is a *JSON object* with the following fields:
    1. `type` — a *JSON string* that determines the type of the tree node.
    2. Other fields depdending on `type`. The following values of `type` are defined (new fields may be
       added in further revisions of the specification):
        1. "`alternatives`" — a list of alternatives (e.g. "`a|b|c`" in the regex).
           Other fields in the *tree node* object:
            1. `alternatives` — a *JSON array* of *spanned tree node* objects. Each of them represents
               one alternative.
        2. "`empty`" — an empty tree node (matches the empty string). No other *tree node* fields are defined.
        3. "`group`" — a group (e.g. "`(foo)`" in the regex).
           May be capturing or non-capturing, see [the reference documentation][cpp.wr22.group]
           for details. Other fields in the tree node object:
            1. `inner` — a *spanned tree node*, which represents the content of the group.
            2. `capture` — a *capture* object, which describes the capturing behavior of this group.
        4. "`literal`" — a literal character (e.g. "`a`").
           Other fields in the *tree node* object:
            1. `char` — a *JSON string* containing 1 character (Unicode codepoint).
               This codepoint is the literal character in the regular expression.
        5. "`optional`", "`plus`", "`star`" — expressions quantified with "`?`", "`+`" and "`*`" respectively.
           Other fields in the *tree node* object:
            1. `inner` — a *spanned tree node* representing the expression under the quantifier.
        6. "`sequence`" — a collection of several subexpressions following each other sequentially
           (e.g. "`[a-z].abc+`" is a sequence of "`[a-z]`", "`.`", "`a`", "`b`" and "`c+`").
           Other fields in the *tree node* object:
            1. `items` — a *JSON array* of *spanned tree nodes*, each representing a subexpression.
        7. "`wildcard`" — a wildcard symbol ("`.`" in the regex). No other *tree node* fields are defined.
        8. "`character_class`" — a character class (e.g. "`[^a-zA-Z_]`").
           Other fields in the *tree node* object:
            1. `inverted` — a *JSON boolean* that is true if the character class's match is inverted (there
               is a `^` in the beginning of the character class) and false otherwise.
               E.g. it is true for the character class "`[^a-z]`" but false for "`[a-z]`".
            2. `ranges` — a *JSON array* of *spanned character range* objects representing the
               sequence of character ranges and individual characters in this character class,
               in the order they are specified in the regex, and together with their spans.
4. **Capture** is a *JSON object* describing the capturing behavior of a group with the following fields:
    1. `type` — a *JSON string* that determines the type of the capture.
    2. Other fields depdending on `type`. The following values of `type` are defined:
        1. "`index`" — group capturing by index (e.g. "`(foo)`"). No other *capture* fields are defined.
        2. "`none`" — non-capturing group (e.g. "`(?:foo)`"). No other *capture* fields are defined.
        3. "`name`" — group capturing by name (e.g. "`(?P<name>foo)`"). Other fields in the *capture* object:
            1. `name` — a *JSON string* representing the name of the capture.
            2. `flavor` — a *JSON string* representing the syntax variant used in the
               regular expression to declare a group capturing by name. Possible values:
                1. "`apostrophes`" — corresponds to "`(?'name'data)`".
                2. "`angles`" — corresponds to "`(?<name>data)`".
                2. "`angles_with_p`" — corresponds to "`(?P<name>data)`".
5. **Span** is a *JSON array* of exactly two *JSON numbers*. Each number is an integer, and the second one
   is greater than or equal to the first one. These numbers represent the starting and ending positions
   of a parse tree node in the regular expression string: the first number is the 0-based index of the first
   Unicode character (codepoint) covered by this tree node, and the second number is the 0-based index of the
   Unicode character right after the last one covered. That is, the left end is included, and the right end
   is excluded. For example, in a regex "`a(b|c)d`" the group "`(b|c)`" has the span `[1, 6]`, because
   the index of "`(`" is 1, and the index of "`d`" (the character right after the group) is 6.
6. **Spanned character range** is a *JSON object* describing a character range and its span in the regex.
It has the following fields:
    1. `range` — the *character range* object representing the range this object describes.
    2. `span` — the *span* of this character range.
7. **Character range** is a *JSON object* describing a single character or a range of characters
   that appear inside a character class. It has the following fields:
    1. `single_char` — a *JSON boolean* that is true if this character range contains only one character
       and false otherwise.
       This corresponds to single characters in character classes (e.g. `[a]`) or trivial ranges
       (e.g. `[a-a]`).
    2. `char` — a *JSON string* of exactly one character, which is the character contained in the range.
       This field is present if and only if `single_char` is true.
    3. `first_char` — a *JSON string* of exactly one character, which is the first character contained
       in the range. This field is present if and only if `single_char` is false.
    4. `last_char` — a *JSON string* of exactly one character, which is the last character contained
       in the range. This field is present if and only if `single_char` is false.

#### `/match`
Match strings against a regular expression.

Allowed HTTP methods: `POST`.

*Request payload* fields:

1. `regex` — A *JSON string* that represents the regular expression to parse. Must always be present.
2. `strings` — A *JSON array* of *string match requests*, each representing a string to match
   against the regular expression. Each **string match request** is a *JSON object* with the following
   fields:
    1. `string` — A *JSON string* representing the string to be matched.
    2. `fragment` — A *JSON string* describing which portion of the string needs to be matched.
       Currently, only the value "`whole`" is supported, which corresponds to matching the entire
       string.

*Response payload* is a *match result* object representing the result of the parse operation.
This and other object types are defined below.

1. **Match result** is a *JSON object*. Two fields are possible, and exactly one must be present:
    1. `match_results` — (if the regular expression provided is parsed correctly)
       a *JSON array*, where each item corresponds to one string in the request's `strings` array.
       Items are *JSON objects* with the following fields:
        1. `algorithm` — a *JSON string* with the name of the algorithm used to match
           this string against the regular expression. Currently, the only possible value is
           "`backtracking`".
        2. `matched` — a *JSON boolean* which is true if the string matches the regex and false otherwise.
        3. `steps` — a *JSON array* of *match steps*, representing the steps the matching algorithm
           has made.
        4. `captures` — (present only if `matched == true`) the captures made by capturing groups.
           It is a *JSON object* with the following fields:
            1. `whole` — a *captured substring* object corresponding to the whole match.
            2. `by_index` — a *JSON object* whose keys are stringified integers (e.g. `"1"`, `"25"`, etc.)
               and values are the *captured substring* objects that correspond to the capturing groups with the
               specified index.
            3. `by_name` — a *JSON object* whose keys are names of named capturing groups
               and values are the *captured substring* objects that correspond to these groups.
    2. `parse_error` — (if the regular expression could not be parsed correctly) the *parse error* object as
       defined in the `/parse` section.
2. **Match step** is a *JSON object*. The only mandatory field is `type`, which is a *JSON string*
   denoting the type of the step. Depending on the value of `type`, the *match step* has other fields.
   The following is the explanation of possible values of `type`:
    1. "`match_literal`" — Expect the next string character to match a given literal.
       Additional fields:
        1. `regex_span` — the *span* of the considered literal in the regex.
        1. `literal` — a *JSON string* of one character with the literal character expected in the string.
        1. `success` — a *JSON boolean* which is true if the string character was the same as the literal
           and false otherwise.
        1. `string_span` — (present only if `success == true`) a *span* object with the span
           of the matched character in the string.
        1. `string_pos` — (present only if `success == false`) an integer *JSON number* describing
           the current position in the string (with 0 meaning "at the beginning", 1 meaning
           "right after the first character" and so on).
        1. `failure_reason` — (present only if `success == false`) a *JSON string* describing
           the reason why the match was not successful. Possible values:
            1. "`other_char`" — the next character in the string was different from the literal in the regex.
            1. "`end_of_input`" — there was no next character in the string (the string ended there).
    1. "`match_wildcard`" — Match any next character of the string.
       Additional fields:
        1. `regex_span` — the *span* of the considered wildcard ("`.`") in the regex.
        1. `success` — a *JSON boolean* which is true if the match was successful and false otherwise.
           For reasons why the match might have failed, see `failure_reason`.
        1. `string_span` — (present only if `success == true`) a *span* object with the span
           of the matched character in the string.
        1. `string_pos` — (present only if `success == false`) an integer *JSON number* describing
           the current position in the string (with 0 meaning "at the beginning", 1 meaning
           "right after the first character" and so on).
        1. `failure_reason` — (present only if `success == false`) a *JSON string* describing
           the reason why the match was not successful. Possible values:
            1. "`end_of_input`" — there was no next character in the string (the string ended there).
    1. "`match_char_class`" — Match the next string character against a character class (a list of
       character ranges and individual characters).
       Additional fields:
        1. `regex_span` — the *span* of the considered character class (e.g. "[a-z]") in the regex.
        1. `success` — a *JSON boolean* which is true if the match was successful and false otherwise.
        1. `string_span` — (present only if `success == true`) a *span* object with the span
           of the matched character in the string.
        1. `string_pos` — (present only if `success == false`) an integer *JSON number* describing
           the current position in the string (with 0 meaning "at the beginning", 1 meaning
           "right after the first character" and so on).
        1. `failure_reason` — (present only if `success == false`) a *JSON string* describing
           the reason why the match was not successful. Possible values:
            1. "`end_of_input`" — there was no next character in the string (the string ended there).
            1. "`excluded_char`" — the next string character did not match the character class.
               E.g. `0` does not match `[a-z]`.
    1. "`match_<quantifier>`" where `<quantifier>` is either of `star`, `plus` or `optional` —
       Start matching the sub-expression under the quantifier repeatedly. By itself, this step
       does not consume any characters from the string and cannot fail.
       Additional fields:
        1. `regex_span` — the *span* of the considered quantified expression (e.g. ".*") in the regex.
        1. `string_pos` — an integer *JSON number* describing
           the current position in the string (with 0 meaning "at the beginning", 1 meaning
           "right after the first character" and so on).
    1. "`finish_<quantifier>`" where `<quantifier>` is either of `star`, `plus` or `optional` —
       Finish repeated matching of the sub-expression under the quantifier and fix the number of repetitions
       matched. Completes the corresponding "`match_<quantifier>`" step. Usually, this step is successful,
       but it can fail if there is no possible number of repetitions that makes the string match
       the regex in the current branch.
       Additional fields:
        1. `regex_span` — the *span* of the considered quantified expression (e.g. ".*") in the regex.
           This is the same expression as in the corresponding `match_<quantifier>` step.
        1. `success` — a *JSON boolean* which is true if the match was successful and false otherwise.
        1. `string_span` — (present only if `success == true`) a *span* object with the overall
           span of all matched repetition of the sub-expression in the string. E.g. it is `[0, 5]`
           for the string "`abcde12345`" (current position is 0) and the regex "[a-z]+".
        1. `num_repetitions` — (present only if `success == true`) an integer *JSON number*,
           the number of repetitions of the sub-expression that were matched.
        1. `string_pos` — (present only if `success == false`) an integer *JSON number* describing
           the current position in the string (with 0 meaning "at the beginning", 1 meaning
           "right after the first character" and so on).
        1. `failure_reason` — (present only if `success == false`) a *JSON string* describing
           the reason why the match was not successful. Possible values:
            1. "`options_exhausted`" — there is no valid number of repetitions that makes the string
               match the regex in the current branch.
    1. "`begin_group`" — Indicates a beginning of a group (sub-expression in parentheses in a regex).
       By itself, this step does not consume any characters from the string and cannot fail.
       Additional fields:
        1. `regex_span` — the *span* of the considered group (e.g. "([0-9]+)") in the regex.
        1. `string_pos` — an integer *JSON number* describing
           the current position in the string (with 0 meaning "at the beginning", 1 meaning
           "right after the first character" and so on).
    1. "`end_group`" — Indicates that a group has finished. Always corresponds to an earlier
       "`begin_group`" step.
       By itself, this step does not consume any characters from the string and cannot fail.
       Additional fields:
        1. `string_pos` — an integer *JSON number* describing
           the current position in the string (with 0 meaning "at the beginning", 1 meaning
           "right after the first character" and so on).
    1. "`match_alternatives`" — Start trying to match alternative subexpressions (e.g. "`abc|[0-9]+`" will
       try to match the string against "`abc`" first, and if it fails, then against "`[0-9]+`").
       By itself, this step does not consume any characters from the string and cannot fail.
       Additional fields:
        1. `regex_span` — the overall *span* of the considered alternatives in the regex.
        1. `string_pos` — an integer *JSON number* describing
           the current position in the string (with 0 meaning "at the beginning", 1 meaning
           "right after the first character" and so on).
    1. "`finish_alternatives`" — make a decision in the choice of an alternative. The current part
       of the string matches this alternative. If, later, this choice will cause the rest of the string
       not to match the rest of the regex, another alternative would be chosen.
       Additional fields:
        1. `regex_span` — the *span* of the considered quantified expression (e.g. ".*") in the regex.
           This is the same expression as in the corresponding `match_<quantifier>` step.
        1. `success` — a *JSON boolean* which is true if the match was successful and false otherwise.
        1. `string_span` — (present only if `success == true`) the *span* of the part of the string
           that matches the chosen alternative.
        1. `alternative_chosen` — (present only if `success == true`) an integer *JSON number*,
           the index of the chosen alternative (starting from 0).
        1. `string_pos` — (present only if `success == false`) an integer *JSON number* describing
           the current position in the string (with 0 meaning "at the beginning", 1 meaning
           "right after the first character" and so on).
        1. `failure_reason` — (present only if `success == false`) a *JSON string* describing
           the reason why the match was not successful. Possible values:
            1. "`options_exhausted`" — there is no valid alternative that makes the string
               match the regex in the current branch.
    1. "`backtrack`" — undo last several steps and go back to try another option, because,
       in this branch, the string does not matches the regex, and there are still other branches
       to try.
       This step may cause the current position in the string to go back, "un-consuming" some characters
       from it. Also, this step cannot fail.
       Additional fields:
        1. `string_pos` — an integer *JSON number* describing
           the position in the string after this step.
        1. `continue_after_step` — an integer *JSON number* representing
           the index of the last step to keep (not to undo). This index
           starts from 0, and the steps are numbered in the same way as they reside
           in the JSON array.
    1. "`end`" — finish matching, successfully or not.
       Additional fields:
        1. `string_pos` — the current position in the string at the end of the matching process.
           If the whole string of length `N` has been matched, `string_pos = N`.
        1. `success` — a *JSON boolean* which is true if the string matches the regex and false otherwise.
1. **Captured substring** is a *span* that corresponds to the position of a captured fragment of the input
   string.

## Service Errors
*Service errors* are represented by *error* objects. The following *error codes* are defined for
*service errors*:

1. "`internal_error`" — due to a bug in the backend, it was unable to handle the request.
   HTTP status code 500 (Internal Server Error) is returned.
   `data` field is absent.
2. "`invalid_request_json`" — the request payload was not a valid JSON document.
   HTTP status code 400 (Bad Request) is returned.
   `data` field is absent.
3. "`invalid_request_json_structure`" — the request payload was valid JSON, but its structure did not
   conform to this specification (e.g. data types were wrong or required fields were missing).
   HTTP status code 400 (Bad Request) is returned.
   `data` field is absent.
4. "`invalid_utf8`" — some strings in the request payload were not correctly UTF-8 encoded. It
   is unspecified when this error is raised in place of "`invalid_request_json`", since the latest
   JSON specification requires the JSON document to be UTF-8 encoded.
   HTTP status code 400 (Bad Request) is returned.
   `data` field is absent.
5. "`not_implemented`" — the requested operation or its part is not implemented on the backend.
   HTTP status code 501 (Not Implemented) is returned.
   `data` field is absent.

## Examples

### Successful Parsing
Request payload:
```json
{
    "regex": "(?P<group>a|b)c"
}
```

Response JSON:
```json
{
    "data": {
        "parse_tree": {
            "span": [0, 15],
            "type": "sequence",
            "items": [
                {
                    "span": [0, 14],
                    "type": "group",
                    "capture": {
                        "type": "name",
                        "name": "group",
                        "flavor": "angles_with_p"
                    },
                    "inner": {
                        "span": [10, 13],
                        "type": "alternatives",
                        "alternatives": [
                            {
                                "span": [10, 11],
                                "type": "literal",
                                "char": "a",
                            },
                            {
                                "span": [12, 13],
                                "type": "literal",
                                "char": "b",
                            }
                        ]
                    }
                },
                {
                    "span": [14, 15],
                    "type": "literal",
                    "char": "c"
                }
            ]
        }
    }
}
```

### Parse Error
Request payload:
```json
{
    "regex": "(text"
}
```

Response JSON:
```json
{
    "data": {
        "parse_error": {
            "code": "unexpected_end",
            "data": {
                "position": 5,
                "expected": "<a description that a closing parenthesis was expected>"
            }
        }
    }
}
```

### Service Error
Request payload:
```json
[1, 2, 3]
```

Response JSON:
```json
{
    "error": {
        "code": "invalid_request_json_structure"
    }
}
```

[cpp.wr22.group]: https://writing-regexps-2021-22.github.io/docs/regex-parser/structwr22_1_1regex__parser_1_1regex_1_1part_1_1Group.html
