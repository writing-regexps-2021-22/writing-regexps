#pragma once
#include <wr22/regex_server/service_error.hpp>

namespace wr22::regex_server::service_error {

constexpr const char not_implemented_code[] = "not_implemented";

/// A service error that indicates a necessary feature not having been implemented.
class NotImplemented : public StaticServiceError<not_implemented_code, 501> {};

}// namespace wr22::regex_server::service_error
