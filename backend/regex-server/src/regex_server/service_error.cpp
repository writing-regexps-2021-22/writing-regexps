#include <wr22/regex_server/service_error.hpp>

namespace wr22::regex_server {

int ServiceError::http_code() const noexcept {
    return 400;
}

const char* ServiceError::what() const noexcept {
    return error_code();
}

}
