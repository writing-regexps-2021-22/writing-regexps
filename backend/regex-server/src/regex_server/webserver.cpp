// wr22
#include <wr22/regex_server/service_error.hpp>
#include <wr22/regex_server/service_error/internal_error.hpp>
#include <wr22/regex_server/service_error/not_implemented.hpp>
#include <wr22/regex_server/webserver.hpp>

// stl
#include <type_traits>

// crow
#include <crow_all.h>

// nlohmann
#include <nlohmann/json.hpp>

// spdlog
#include <spdlog/spdlog.h>

namespace wr22::regex_server {

namespace {
    /// Pointer to member of `Webserver`.
    using HandlerPtr =
        nlohmann::json (Webserver::*)(const crow::request& request, crow::response& response);

    void write_success_response(crow::response& response, nlohmann::json data) {
        auto response_json = nlohmann::json::object();
        response_json["data"] = std::move(data);
        response.body = response_json.dump();
        response.code = 200;
        response.end();
    }

    void write_error_response(crow::response& response, const ServiceError& service_error) {
        auto error_json = nlohmann::json::object();
        error_json["code"] = service_error.error_code();

        auto response_json = nlohmann::json::object();
        response_json["error"] = std::move(error_json);

        response.body = response_json.dump();
        response.code = service_error.http_code();
        response.end();
    }

    /// Wrap a request handler to respond with appropriate messages in case of success or failure.
    auto handle_errors_in(Webserver& webserver, HandlerPtr func) {
        return [func, &webserver](const crow::request& request, crow::response& response) {
            try {
                auto response_data = (webserver.*func)(request, response);
                write_success_response(response, std::move(response_data));
            } catch (const ServiceError& error) {
                SPDLOG_WARN("Service error: {}", error.what());
                write_error_response(response, error);
            } catch (const std::exception& e) {
                SPDLOG_ERROR("Unhandled exception during handling a request: {}", e.what());
                write_error_response(response, service_error::InternalError{});
            }
        };
    }

}  // namespace

Webserver::Webserver() {
    CROW_ROUTE(m_app, "/parse")
        .methods(crow::HTTPMethod::POST)(handle_errors_in(*this, &Webserver::parse_handler));
}

void Webserver::run() {
    m_app.loglevel(crow::LogLevel::Warning).port(6666).bindaddr("127.0.0.1").run();
}

nlohmann::json Webserver::parse_handler(
    [[maybe_unused]] const crow::request& request,
    crow::response& response) {
    throw service_error::NotImplemented{};
}

}  // namespace wr22::regex_server
