#pragma once

// wr22
#include <wr22/regex_server/json_result.hpp>

// crow
#include <crow_all.h>

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
    void parse_handler(const crow::request& request, crow::response& response);

    crow::SimpleApp m_app;
};

}  // namespace wr22::regex_server
