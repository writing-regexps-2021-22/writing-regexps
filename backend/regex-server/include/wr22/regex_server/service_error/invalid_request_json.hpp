#pragma once
#include <wr22/regex_server/service_error.hpp>

namespace wr22::regex_server::service_error {

constexpr const char invalid_request_json_code[] = "invalid_request_json";

/// A service error that indicates a necessary feature not having been implemented.
class InvalidRequestJson : public StaticServiceError<invalid_request_json_code, 400> {};

}// namespace wr22::regex_server::service_error
