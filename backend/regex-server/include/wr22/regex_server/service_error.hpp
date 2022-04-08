#pragma once
#include <exception>

namespace wr22::regex_server {

/// Base class for all errors that prevent a client's request from being handled.
/// Note that it does not cover errors that are a possible result of executing
/// the request. For example, a regex with invalid syntax in the request does not constitute a
/// service error, while a regex missing from the request completely does.
///
/// Errors are identified by their codes, which are unique machine-friendly strings.
/// For specific error types, see the `wr22::regex_server::service_error` namespace.
/// Also, errors may have a custom descriptive HTTP status codes associated with them.
/// If an error is thrown from a request handler, the HTTP status code is set to this value,
/// if it is overriden, or to the generic 400 (Bad Request) otherwise.
class ServiceError : public std::exception {
public:
    /// Get the error code. Must be implemented by subclasses.
    virtual const char* error_code() const noexcept = 0;

    /// Get the error HTTP status code. The default implementation returns 400 (Bad Request).
    virtual int http_code() const noexcept;

    const char* what() const noexcept override;
};

/// Base class for service errors whose error code and HTTP status codes are known and the same for all instances of the
/// respective error type. This class exists to simplify creation of new error types. For example, instead
/// of defining a new error type as:
///
/// ```cpp
/// class MyError : public ServiceError {
/// public:
///     const char* error_code() const noexcept override {
///         return "my_error";
///     }
///
///     int http_code() const noexcept override {
///         return 451;
///     }
/// };
/// ```
///
/// one can define it as follows:
///
/// ```cpp
/// // https://stackoverflow.com/a/1826505 explains how to use string literals
/// // in template arguments properly.
/// constexpr const char my_error_code[] = "my_error";
/// class MyError : public StaticServiceError<my_error_code, 451> {};
/// ```
///
/// Please note that simply defining a type alias as follows:
///
/// ```cpp
/// using MyError = StaticServiceError<my_error_code, 451>;
/// ```
///
/// is technically possible but not recommended for various reasons. These include consistency with
/// existing code and the general principle of creating distinct types for distinct entities.
template <const char* error_code_literal, int http_code_literal = 400>
class StaticServiceError : public ServiceError {
public:
    const char* error_code() const noexcept override {
        return error_code_literal;
    }

    int http_code() const noexcept override {
        return http_code_literal;
    }
};

}  // namespace wr22::regex_server
