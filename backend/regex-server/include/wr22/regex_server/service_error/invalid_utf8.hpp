#pragma once
#include <wr22/regex_server/service_error.hpp>

namespace wr22::regex_server::service_error {

constexpr const char invalid_utf8_code[] = "invalid_utf8";

/// A service error that indicates a necessary feature not having been implemented.
class InvalidUtf8 : public StaticServiceError<invalid_utf8_code, 400> {};

}// namespace wr22::regex_server::service_error
