#pragma once

// crow
#include <crow.h>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_server {

class Webserver {
public:
    Webserver();
    Webserver(const Webserver& other) = delete;
    Webserver(Webserver&& other) = delete;
    Webserver& operator=(const Webserver& other) = delete;
    Webserver& operator=(Webserver&& other) = delete;

    void run();

private:
    nlohmann::json parse_handler(const crow::request& request, crow::response& response);
    nlohmann::json explain_handler(const crow::request& request, crow::response& response);

    crow::SimpleApp m_app;
};

}  // namespace wr22::regex_server
