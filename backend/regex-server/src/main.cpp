#define CROW_MAIN

// wr22
#include <wr22/regex_server/webserver.hpp>

// spdlog
#include <spdlog/spdlog.h>

int main() {
    try {
        auto webserver = wr22::regex_server::Webserver();
        // TODO: make use of non-blocking methods if necessary.
        webserver.run();
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Unhandled exception: {}", e.what());
    }
}
