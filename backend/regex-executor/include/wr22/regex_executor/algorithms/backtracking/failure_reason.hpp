#pragma once

// stl
#include <type_traits>

// wr22
#include <wr22/utils/adt.hpp>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

namespace failure_reasons {
    struct EndOfInput {
        static constexpr const char* code() {
            return "end_of_input";
        }

        bool operator==(const EndOfInput& other) const = default;
    };

    struct ExcludedChar {
        static constexpr const char* code() {
            return "excluded_char";
        }

        bool operator==(const ExcludedChar& other) const = default;
    };

    struct OtherChar {
        static constexpr const char* code() {
            return "other_char";
        }

        bool operator==(const OtherChar& other) const = default;
    };

    struct OptionsExhausted {
        static constexpr const char* code() {
            return "options_exhausted";
        }

        bool operator==(const OptionsExhausted& other) const = default;
    };
}  // namespace failure_reasons

template <typename... Reasons>
struct FailureReason {
    using Adt = wr22::utils::Adt<Reasons...>;

    template <typename Reason>
    FailureReason(Reason reason)
        : reason(std::move(reason)) {}

    Adt reason;

    bool operator==(const FailureReason<Reasons...>& other) const = default;
};

template <typename... Reasons>
void to_json(nlohmann::json& j, const FailureReason<Reasons...>& failure_reason) {
    j = nlohmann::json::object();
    failure_reason.reason.visit([&j](const auto& reason) { j["code"] = reason.code(); });
}

}  // namespace wr22::regex_executor::algorithms::backtracking
