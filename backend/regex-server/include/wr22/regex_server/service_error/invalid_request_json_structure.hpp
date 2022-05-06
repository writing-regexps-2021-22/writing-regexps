#pragma once
#include <wr22/regex_server/service_error.hpp>

namespace wr22::regex_server::service_error {

constexpr const char invalid_request_json_structure_code[] = "invalid_request_json_structure";

/// A service error that indicates a necessary feature not having been implemented.
class InvalidRequestJsonStructure :
    public StaticServiceError<invalid_request_json_structure_code, 400> {};

}  // namespace wr22::regex_server::service_error
