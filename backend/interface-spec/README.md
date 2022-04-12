# Communication Interface Specification v 0.1.0

***NOTE:*** this document is a preliminary draft of what is going to be a specification.
Many details are still missing and no examples have been provided yet. This is going
to be corrected soon.

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

If present, the `error` field is an **error object**. The latter is defined as a *JSON object*
with the following fields:

1. `code` — A *JSON string* that represents the error code. Must always be present.
2. `data` — A *JSON value* of the error-specific format. The precise format is defined together with
   the definition of the corresponding error code. If no definition of the `data` field is given for an
   error code, this field must be absent if such error is returned.

The list of defined request paths is as follows:

1. `/parse`

### Request Paths
#### `/parse`
Parse a regular expression into its syntax tree.

Allowed HTTP methods: `POST`.

*Request payload* fields:

1. `regex` — A JSON string that represents the regular expression to parse. Must always be present.

*Response payload* is a *spanned tree node* object corresponding to the root parse tree node.
This and other object types are defined below.

(TODO: check this)

1. **Spanned tree node** is a *JSON object* with the following fields:
    1. `span` — a *span*. Determines the starting and ending positions of the current
       parse tree node in the regular expression string.
    2. All fields from a *tree node* object.
2. **Tree node** is a *JSON object* with the following fields:
    1. `type` — a *JSON string* that determines the type of the tree node.
    2. Other fields depdending on `type`. The following values of `type` are defined:
        1. `alternatives` — a list of alternatives (e.g. "`a|b|c`" in the regex).
           Other fields in the *tree node* object:
            1. `alternatives` — a *JSON array* of *spanned tree node* objects. Each of them represents
               one alternative.
        2. `empty` — an empty tree node (matches the empty string). No other *tree node* fields are defined.
        3. `group` — a group (e.g. "`(foo)`" in the regex).
           May be capturing or non-capturing, see [the reference documentation][cpp.wr22.group]
           for details. Other fields in the tree node object:
            1. `inner` — a *spanned tree node*, which represents the content of the group.
            2. `capture` — a *capture* object, which describes the capturing behavior of this group.
        4. `literal` — a literal character (e.g. "`a`").
           Other fields in the *tree node* object:
            1. `char` — a *JSON string* containing 1 character (Unicode codepoint).
               This codepoint is the literal character in the regular expression.
        5. `optional`, `plus`, `star` — expressions quantified with `?`, `+` and `*` respectively.
           Other fields in the *tree node* object:
            1. `inner` — a *spanned tree node* representing the expression under the quantifier.
        6. `sequence` — a collection of several subexpressions following each other sequentially
           (e.g. "`[a-z].abc+`" is a sequence of "`[a-z]`", "`.`", "`a`", "`b`" and "`c+`").
           Other fields in the *tree node* object:
            1. `items` — a *JSON array* of *spanned tree nodes*, each representing a subexpression.
        7. `wildcard` — a wildcard symbol ("`.`" in the regex). No other *tree node* fields are defined.
3. **Capture** is a *JSON object* describing the capturing behavior of a group with the following fields:
    1. `type` — a *JSON string* that determines the type of the capture.
    2. Other fields depdending on `type`. The following values of `type` are defined:
        1. `index` — group capturing by index (e.g. "`(foo)`"). No other *capture* fields are defined.
        2. `none` — non-capturing group (e.g. "`(?:foo)`"). No other *capture* fields are defined.
        3. `name` — group capturing by name (e.g. "`(?P<name>foo)`"). Other fields in the *capture* object:
            1. `name` — a *JSON string* representing the name of the capture.
            2. `flavor` — a *JSON string* representing the syntax variant used in the
               regular expression to declare a group capturing by name. Possible values:
                1. `apostrophes` — corresponds to "`(?'name'data)`".
                2. `angles` — corresponds to "`(?<name>data)`".
                2. `angles_with_p` — corresponds to "`(?P<name>data)`".
4. **Span** is a *JSON array* of exactly two *JSON numbers*. Each number is an integer, and the second one
   is greater than or equal to the first one. These numbers represent the starting and ending positions
   of a parse tree node in the regular expression string: the first number is the 0-based index of the first
   Unicode character (codepoint) covered by this tree node, and the second number is the 0-based index of the
   Unicode character right after the last one covered. That is, the left end is included, and the right end
   is excluded. For example, in a regex "`a(b|c)d`" the group "`(b|c)`" has the span `[1, 6]`, because
   the index of "`(`" is 1, and the index of "`d`" (the character right after the group) is 6.

### Errors
TODO

## Service Errors
TODO

[cpp.wr22.group]: https://writing-regexps-2021-22.github.io/docs/regex-parser/structwr22_1_1regex__parser_1_1regex_1_1part_1_1Group.html
