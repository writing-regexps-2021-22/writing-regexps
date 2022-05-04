# Communication Interface Specification v 0.1.0

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
    2. `parse_error` — an *error* object describing the parse error if it has occurred.
       The possible *error codes* are defined as follows:
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
