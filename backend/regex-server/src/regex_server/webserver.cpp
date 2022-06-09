// wr22
#include <wr22/regex_executor/executor.hpp>
#include <wr22/regex_executor/regex.hpp>
#include <wr22/regex_explainer/explanation/explanation.hpp>
#include <wr22/regex_explainer/hints/hint.hpp>
#include <wr22/regex_parser/parser/errors.hpp>
#include <wr22/regex_parser/parser/regex.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/regex_server/service_error.hpp>
#include <wr22/regex_server/service_error/internal_error.hpp>
#include <wr22/regex_server/service_error/invalid_request_json.hpp>
#include <wr22/regex_server/service_error/invalid_request_json_structure.hpp>
#include <wr22/regex_server/service_error/invalid_utf8.hpp>
#include <wr22/regex_server/service_error/not_implemented.hpp>
#include <wr22/regex_server/webserver.hpp>
#include <wr22/unicode/conversion.hpp>

// stl
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>

// crow
#include <crow.h>

// nlohmann
#include <nlohmann/json.hpp>

// spdlog
#include <spdlog/spdlog.h>

// fmt
#include <fmt/format.h>

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

    struct ParseSuccess {
        regex_parser::regex::SpannedPart part;
    };
    struct ParseFailure {
        nlohmann::json parse_error;
    };

    utils::Adt<ParseSuccess, ParseFailure> parse_regex(std::u32string_view regex) {
        namespace err = wr22::regex_parser::parser::errors;

        auto error_code = "";
        auto error_data = nlohmann::json::object();
        try {
            return ParseSuccess{wr22::regex_parser::parser::parse_regex(regex)};
        } catch (const err::ExpectedEnd& e) {
            error_code = "expected_end";
            error_data["position"] = e.position();
            error_data["char_got"] = wr22::unicode::to_utf8(e.char_got());
            error_data["hint"] = regex_explainer::hints::get_hint(e);
        } catch (const err::UnexpectedChar& e) {
            error_code = "unexpected_char";
            error_data["position"] = e.position();
            error_data["char_got"] = wr22::unicode::to_utf8(e.char_got());
            error_data["expected"] = e.expected();
            error_data["hint"] = regex_explainer::hints::get_hint(e);
            if (auto c = e.needs_closing(); c.has_value()) {
                error_data["needs_closing"] = wr22::unicode::to_utf8(c.value());
            }
        } catch (const err::UnexpectedEnd& e) {
            error_code = "unexpected_end";
            error_data["position"] = e.position();
            error_data["expected"] = e.expected();
            error_data["hint"] = regex_explainer::hints::get_hint(e);
            if (auto c = e.needs_closing(); c.has_value()) {
                error_data["needs_closing"] = wr22::unicode::to_utf8(c.value());
            }
        } catch (const err::InvalidRange& e) {
            error_code = "invalid_range";
            error_data["span"] = e.span();
            error_data["first"] = wr22::unicode::to_utf8(e.first());
            error_data["last"] = wr22::unicode::to_utf8(e.last());
            error_data["hint"] = regex_explainer::hints::get_hint(e);
        } catch (const err::TooStronglyNested&) {
            error_code = "too_strongly_nested";
        } catch (const err::ParseError& e) {
            throw std::runtime_error(fmt::format("Unknown parse error: {}", e.what()));
        }
        // If here, an error has occurred.
        auto parse_error_json = nlohmann::json::object();
        parse_error_json["code"] = error_code;
        parse_error_json["data"] = std::move(error_data);

        return ParseFailure{std::move(parse_error_json)};
    }

    nlohmann::json parse_regex_to_json(const std::u32string_view& regex) {
        return std::move(parse_regex(regex))
            .visit(
                [](const ParseSuccess& success) -> nlohmann::json {
                    auto data_json = nlohmann::json::object();
                    data_json["parse_tree"] = success.part;
                    return data_json;
                },
                [](ParseFailure failure) -> nlohmann::json {
                    auto data_json = nlohmann::json::object();
                    data_json["parse_error"] = std::move(failure.parse_error);
                    return data_json;
                });
    }

    std::u32string decode_string(const std::string& string) {
        try {
            return wr22::unicode::from_utf8(string);
        } catch (const boost::locale::conv::conversion_error& e) {
            throw service_error::InvalidUtf8{};
        }
    }

    std::string extract_json_string(const nlohmann::json& json) {
        if (!json.is_string()) {
            throw service_error::InvalidRequestJsonStructure{};
        }
        return json.get<std::string>();
    }

    std::u32string decode_json_string(const nlohmann::json& json) {
        return decode_string(extract_json_string(json));
    }

    const nlohmann::json& json_at(const nlohmann::json& json, const char* key) {
        if (!json.is_object()) {
            throw service_error::InvalidRequestJsonStructure{};
        }
        if (auto it = json.find(key); it != json.end()) {
            return *it;
        }
        throw service_error::InvalidRequestJsonStructure{};
    }
}  // namespace

Webserver::Webserver() {
    CROW_ROUTE(m_app, "/parse")
        .methods(crow::HTTPMethod::POST)(handle_errors_in(*this, &Webserver::parse_handler));
    CROW_ROUTE(m_app, "/explain")
        .methods(crow::HTTPMethod::POST)(handle_errors_in(*this, &Webserver::explain_handler));
    CROW_ROUTE(m_app, "/match")
        .methods(crow::HTTPMethod::POST)(handle_errors_in(*this, &Webserver::match_handler));
}

void Webserver::run() {
    m_app.loglevel(crow::LogLevel::Warning).port(6666).bindaddr("127.0.0.1").run();
}

nlohmann::json Webserver::parse_handler(const crow::request& request, crow::response& response) {
    const auto request_json = nlohmann::json::parse(request.body, nullptr, false);
    if (request_json.is_discarded()) {
        throw service_error::InvalidRequestJson{};
    }

    auto regex = decode_json_string(json_at(request_json, "regex"));
    return parse_regex_to_json(regex);
}

nlohmann::json Webserver::match_handler(const crow::request& request, crow::response& response) {
    const auto request_json = nlohmann::json::parse(request.body, nullptr, false);
    if (request_json.is_discarded()) {
        throw service_error::InvalidRequestJson{};
    }

    auto regex_string = decode_json_string(json_at(request_json, "regex"));
    auto json_strings = json_at(request_json, "strings");
    if (!json_strings.is_array()) {
        throw service_error::InvalidRequestJson{};
    }

    // TODO: handle parse errors.
    return parse_regex(regex_string)
        .visit(
            [&](ParseSuccess& success) {
                auto root_part = std::move(success.part);
                auto regex = regex_executor::Regex(std::move(root_part));
                auto response_json = nlohmann::json::object();
                auto& match_results = response_json["match_results"];
                match_results = nlohmann::json::array();
                auto executor = regex_executor::Executor(regex);

                for (const auto& json_string_spec : json_strings) {
                    auto string = decode_json_string(json_at(json_string_spec, "string"));
                    auto fragment_string = extract_json_string(
                        json_at(json_string_spec, "fragment"));
                    if (fragment_string != "whole") {
                        // STUB.
                        throw service_error::NotImplemented{};
                    }

                    auto result = executor.execute(string);
                    match_results.push_back(std::move(result));
                }
                return response_json;
            },
            [](ParseFailure& failure) {
                auto response_json = nlohmann::json::object();
                response_json["parse_error"] = std::move(failure.parse_error);
                return response_json;
            });
}

nlohmann::json Webserver::explain_handler(
    [[maybe_unused]] const crow::request& request,
    crow::response& response) {
    const auto request_json = nlohmann::json::parse(request.body, nullptr, false);
    if (request_json.is_discarded()) {
        throw service_error::InvalidRequestJson{};
    }

    if (!request_json.is_object()) {
        throw service_error::InvalidRequestJsonStructure{};
    }

    if (auto it = request_json.find("regex"); it != request_json.end()) {
        const auto& regex_json_string = *it;
        if (!regex_json_string.is_string()) {
            throw service_error::InvalidRequestJsonStructure{};
        }

        auto regex_string = regex_json_string.get<std::string>();
        try {
            auto regex_string_utf32 = wr22::unicode::from_utf8(regex_string);
            return parse_regex(regex_string_utf32)
                .visit(
                    [](const ParseSuccess& success) {
                        const auto& root_part = success.part;
                        auto full_explanation = wr22::regex_explainer::explanation::
                            get_full_explanation(root_part);
                        auto response_json = nlohmann::json::object();
                        response_json["explanation"] = full_explanation;
                        return response_json;
                    },
                    [](ParseFailure failure) {
                        auto response_json = nlohmann::json::object();
                        response_json["parse_error"] = std::move(failure.parse_error);
                        return response_json;
                    });
        } catch (const boost::locale::conv::conversion_error& e) {
            throw service_error::InvalidUtf8{};
        }
    } else {
        throw service_error::InvalidRequestJsonStructure{};
    }
}

}  // namespace wr22::regex_server
