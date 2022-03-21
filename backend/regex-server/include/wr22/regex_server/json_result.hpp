#pragma once

// crow
#include <crow_all.h>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_server {

class JsonResult : public crow::returnable {
public:
    JsonResult(nlohmann::json json);
    std::string dump() const override;

private:
    nlohmann::json m_json;
};

}  // namespace wr22::regex_server
