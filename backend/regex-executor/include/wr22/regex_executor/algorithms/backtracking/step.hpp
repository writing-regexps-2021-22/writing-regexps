#pragma once

// wr22
#include <wr22/regex_executor/quantifier_type.hpp>
#include <wr22/regex_parser/span/span.hpp>
#include <wr22/utils/adt.hpp>

// stl
#include <string>
#include <variant>

// nlohmann
#include <nlohmann/json.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

// TODO: maybe move to a separate file.
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
};  // namespace failure_reasons

namespace detail::step {
    struct SuccessBase {
        static constexpr bool success = true;
        bool operator==(const SuccessBase& other) const = default;
    };

    struct FailureBase {
        static constexpr bool success = false;
        bool operator==(const FailureBase& other) const = default;
    };

}  // namespace detail::step

namespace step {
    struct MatchQuantifier {
        regex_parser::span::Span regex_span;
        size_t string_pos;
        QuantifierType quantifier_type;

        const char* type_code() const;
        bool operator==(const MatchQuantifier& other) const = default;
    };
    void to_json(nlohmann::json& j, const MatchQuantifier& step);

    struct FinishQuantifier {
        QuantifierType quantifier_type;
        regex_parser::span::Span regex_span;
        struct Success : public detail::step::SuccessBase {
            regex_parser::span::Span string_span;
            size_t num_repetitions;
        };
        struct Failure : public detail::step::FailureBase {
            size_t string_pos;
            utils::Adt<failure_reasons::OptionsExhausted> failure_reason;
        };
        utils::Adt<Success, Failure> result;

        const char* type_code() const;
        bool operator==(const FinishQuantifier& other) const = default;
    };
    void to_json(nlohmann::json& j, const FinishQuantifier& step);

    struct MatchCharClass {
        regex_parser::span::Span regex_span;
        struct Success : public detail::step::SuccessBase {
            regex_parser::span::Span string_span;
        };
        struct Failure : public detail::step::FailureBase {
            size_t string_pos;
            utils::Adt<failure_reasons::ExcludedChar, failure_reasons::EndOfInput> failure_reason;
        };
        utils::Adt<Success, Failure> result;

        constexpr const char* type_code() const {
            return "match_char_class";
        }
        bool operator==(const MatchCharClass& other) const = default;
    };
    void to_json(nlohmann::json& j, const MatchCharClass& step);

    struct MatchWildcard {
        regex_parser::span::Span regex_span;
        struct Success : public detail::step::SuccessBase {
            regex_parser::span::Span string_span;
        };
        struct Failure : public detail::step::FailureBase {
            size_t string_pos;
            utils::Adt<failure_reasons::EndOfInput> failure_reason;
        };
        utils::Adt<Success, Failure> result;

        constexpr const char* type_code() const {
            return "match_wildcard";
        }
        bool operator==(const MatchWildcard& other) const = default;
    };
    void to_json(nlohmann::json& j, const MatchWildcard& step);

    struct BeginGroup {
        // TODO: captures.
        regex_parser::span::Span regex_span;
        size_t string_pos;

        constexpr const char* type_code() const {
            return "begin_group";
        }
        bool operator==(const BeginGroup& other) const = default;
    };
    void to_json(nlohmann::json& j, const BeginGroup& step);

    struct EndGroup {
        size_t string_pos;
        constexpr const char* type_code() const {
            return "end_group";
        }
        bool operator==(const EndGroup& other) const = default;
    };
    void to_json(nlohmann::json& j, const EndGroup& step);

    struct MatchLiteral {
        regex_parser::span::Span regex_span;
        char32_t literal;
        struct Success : public detail::step::SuccessBase {
            regex_parser::span::Span string_span;
        };
        struct Failure : public detail::step::FailureBase {
            size_t string_pos;
            utils::Adt<failure_reasons::OtherChar, failure_reasons::EndOfInput> failure_reason;
        };
        utils::Adt<Success, Failure> result;

        constexpr const char* type_code() const {
            return "match_literal";
        }
        bool operator==(const MatchLiteral& other) const = default;
    };
    void to_json(nlohmann::json& j, const MatchLiteral& step);

    struct MatchAlternatives {
        regex_parser::span::Span regex_span;
        size_t string_pos;

        constexpr const char* type_code() const {
            return "match_alternatives";
        }
        bool operator==(const MatchAlternatives& other) const = default;
    };
    void to_json(nlohmann::json& j, const MatchAlternatives& step);

    struct FinishAlternatives {
        regex_parser::span::Span regex_span;
        struct Success : public detail::step::SuccessBase {
            regex_parser::span::Span string_span;
            size_t alternative_chosen;
        };
        struct Failure : public detail::step::FailureBase {
            size_t string_pos;
            utils::Adt<failure_reasons::OptionsExhausted> failure_reason;
        };
        utils::Adt<Success, Failure> result;

        constexpr const char* type_code() const {
            return "finish_alternatives";
        }
        bool operator==(const FinishAlternatives& other) const = default;
    };
    void to_json(nlohmann::json& j, const FinishAlternatives& step);

    struct Backtrack {
        struct Origin {
            size_t step;
            bool operator==(const Origin& other) const = default;
        };
        struct ReconsideredDecision {
            size_t step;
            bool operator==(const ReconsideredDecision& other) const = default;
        };

        regex_parser::span::Span regex_span;
        size_t string_pos;
        Origin origin;
        ReconsideredDecision reconsidered_decision;
        size_t continue_after_step;

        constexpr const char* type_code() const {
            return "backtrack";
        }
        bool operator==(const Backtrack& other) const = default;
    };
    void to_json(nlohmann::json& j, const Backtrack& step);

    struct End {
        size_t string_pos;
        struct Success : public detail::step::SuccessBase {};
        struct Failure : public detail::step::FailureBase {};

        constexpr const char* type_code() const {
            return "end";
        }
        bool operator==(const End& other) const = default;
    };
    void to_json(nlohmann::json& j, const End& step);

    using Adt = utils::Adt<
        MatchQuantifier,
        FinishQuantifier,
        MatchCharClass,
        MatchLiteral,
        MatchWildcard,
        BeginGroup,
        EndGroup,
        MatchAlternatives,
        FinishAlternatives,
        Backtrack,
        End>;
}  // namespace step

struct Step : public step::Adt {
    using step::Adt::Adt;
};
void to_json(nlohmann::json& j, const Step& step);

}  // namespace wr22::regex_executor::algorithms::backtracking
