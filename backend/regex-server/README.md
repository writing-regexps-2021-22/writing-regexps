# regex-server
The main backend application and entry point.
Runs an HTTP server on 127.0.0.1:6666 and handles API requests for operations with regular
expressions. The following operations are implemented:

1. Parse a regular expression into its syntax tree.
Request path: `/parse`.
Request method: POST.
Request body: `{"regex": "<regular expression>"}`

For additional information on the API interface and usage examples, see the
[Communication Interface Specification](https://writing-regexps-2021-22.github.io/docs/interface-spec/readme.html).
