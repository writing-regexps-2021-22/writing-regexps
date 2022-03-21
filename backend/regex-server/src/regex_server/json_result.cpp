// wr22
#include <wr22/regex_server/json_result.hpp>

namespace wr22::regex_server {

JsonResult::JsonResult(nlohmann::json json)
    : crow::returnable("application/json"), m_json(std::move(json)) {}

std::string JsonResult::dump() const {
    return m_json.dump();
}

}  // namespace wr22::regex_server
