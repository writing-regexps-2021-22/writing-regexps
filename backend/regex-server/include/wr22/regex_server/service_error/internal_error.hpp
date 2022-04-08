#pragma once
#include <wr22/regex_server/service_error.hpp>

namespace wr22::regex_server::service_error {

constexpr const char internal_error_code[] = "internal_error";

/// A service error that indicates an unexpected internal error in the backend implementation.
class InternalError : public StaticServiceError<internal_error_code, 500> {};

}  // namespace wr22::regex_server::service_error
