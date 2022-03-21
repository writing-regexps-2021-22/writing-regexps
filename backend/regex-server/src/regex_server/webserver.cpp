// wr22
#include <crow_all.h>
#include <wr22/regex_server/webserver.hpp>

namespace wr22::regex_server {

Webserver::Webserver() {
    CROW_ROUTE(m_app, "/parse")
        .methods(crow::HTTPMethod::POST)(
            [this](const crow::request& request, crow::response& response) {
                parse_handler(request, response);
            });
}

void Webserver::run() {
    m_app.loglevel(crow::LogLevel::Warning).port(6666).bindaddr("127.0.0.1").run();
}

void Webserver::parse_handler(
    [[maybe_unused]] const crow::request& request,
    crow::response& response) {
    auto response_json = nlohmann::json(
        {{"error",
          {
              {"code", "not_implemented"},
              {"message", "Serializing parse tree to JSON is not yet implemented"},
          }}});
    response.code = 501;
    response.body = response_json.dump();
    response.end();
}

}  // namespace wr22::regex_server
