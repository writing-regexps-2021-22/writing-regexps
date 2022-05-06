#pragma once

// wr22
#include <wr22/regex_executor/quantifier_type.hpp>
#include <wr22/regex_parser/span/span.hpp>

// stl
#include <string>
#include <variant>

namespace wr22::regex_executor::algorithms::backtracking {

// TODO: maybe move to a separate file.
namespace failure_reasons {
    struct EndOfInput {
        static constexpr const char* code() {
            return "end_of_input";
        }
    };

    struct ExcludedChar {
        static constexpr const char* code() {
            return "excluded_char";
        }
    };

    struct OtherChar {
        static constexpr const char* code() {
            return "other_char";
        }
    };

    struct OptionsExhausted {
        static constexpr const char* code() {
            return "options_exhausted";
        }
    };
};  // namespace failure_reasons

namespace detail::step {
    struct SuccessBase {
        static constexpr bool success = true;
    };

    struct FailureBase {
        static constexpr bool success = false;
    };

    namespace has {
        struct StringPos {
            size_t string_pos;
        };

        struct RegexSpan {
            regex_parser::span::Span regex_span;
        };

        struct StringSpan {
            regex_parser::span::Span string_span;
        };
    }  // namespace has

    struct DefaultCommon : public has::RegexSpan {};

    struct DefaultSuccess : public has::StringSpan {};

    template <typename... FailureVariants>
    struct DefaultFailure : public has::StringPos {
        using FailureReason = std::variant<FailureVariants...>;
        FailureReason failure_reason;
    };

    struct DefaultInfallible : public has::StringPos, public has::RegexSpan {};

    template <typename CommonT, typename SuccessT, typename FailureT>
    struct FallibleStep : public CommonT {
        struct Success : public SuccessT, public SuccessBase {};
        struct Failure : public FailureT, public FailureBase {};
        using Result = std::variant<Success, Failure>;
        Result result;
    };

    template <typename T, const char* type_code_v>
    struct WithTypeCode : public T {
        static constexpr const char* type_code() {
            return type_code_v;
        }
    };

    namespace variant_pieces {
        namespace match_quantifier {
            struct Infallible : public DefaultInfallible {
                QuantifierType quantifier_type;
            };
            using Step = Infallible;
        }  // namespace match_quantifier

        namespace finish_quantifier {
            struct Common {
                QuantifierType quantifier_type;
            };
            struct Success : public DefaultSuccess {
                size_t num_repetitions;
            };
            struct Failure : public DefaultFailure<failure_reasons::OptionsExhausted> {};
            using Step = FallibleStep<Common, Success, Failure>;
        }  // namespace finish_quantifier

        namespace match_char_class {
            struct Common : public DefaultCommon {};
            struct Success : public DefaultSuccess {};
            struct Failure :
                public DefaultFailure<failure_reasons::ExcludedChar, failure_reasons::EndOfInput> {
            };
            constexpr char type_code[] = "match_char_class";
            using Step = WithTypeCode<FallibleStep<Common, Success, Failure>, type_code>;
        }  // namespace match_char_class

        namespace match_wildcard {
            struct Common : public DefaultCommon {};
            struct Success : public DefaultSuccess {};
            struct Failure : public DefaultFailure<failure_reasons::EndOfInput> {};
            constexpr char type_code[] = "match_wildcard";
            using Step = WithTypeCode<FallibleStep<Common, Success, Failure>, type_code>;
        }  // namespace match_wildcard

        namespace begin_group {
            struct Infallible : public DefaultInfallible {
                // TODO: capture.
                regex_parser::span::Span regex_span;
            };
            constexpr char type_code[] = "begin_group";
            using Step = WithTypeCode<Infallible, type_code>;
        }  // namespace begin_group

        namespace end_group {
            struct Infallible : public DefaultInfallible {};
            constexpr char type_code[] = "end_group";
            using Step = WithTypeCode<Infallible, type_code>;
        }  // namespace end_group

        namespace match_literal {
            struct Common : public DefaultCommon {
                std::u32string literal;
            };
            struct Success : public DefaultSuccess {};
            struct Failure :
                public DefaultFailure<failure_reasons::OtherChar, failure_reasons::EndOfInput> {};
            constexpr char type_code[] = "match_literal";
            using Step = WithTypeCode<FallibleStep<Common, Success, Failure>, type_code>;
        }  // namespace match_literal

        namespace match_alternatives {
            struct Infallible : public DefaultInfallible {};
            constexpr char type_code[] = "match_alternatives";
            using Step = WithTypeCode<Infallible, type_code>;
        }  // namespace match_alternatives

        namespace finish_alternatives {
            struct Common : public DefaultCommon {};
            struct Success : public DefaultSuccess {
                size_t alternative_chosen;
            };
            struct Failure : public DefaultFailure<failure_reasons::OptionsExhausted> {};
            constexpr char type_code[] = "finish_alternatives";
            using Step = WithTypeCode<FallibleStep<Common, Success, Failure>, type_code>;
        }  // namespace finish_alternatives

        namespace backtrack {
            struct Infallible : public DefaultInfallible {
                struct Origin {
                    size_t step;
                };
                struct ReconsideredDecision {
                    size_t step;
                };

                Origin origin;
                ReconsideredDecision reconsidered_decision;
                size_t continue_after_step;
            };
            constexpr char type_code[] = "backtrack";
            using Step = WithTypeCode<Infallible, type_code>;
        }  // namespace backtrack

        namespace end {
            struct Common : public has::StringPos {};
            struct Success {};
            struct Failure {};
            constexpr char type_code[] = "end";
            using Step = WithTypeCode<FallibleStep<Common, Success, Failure>, type_code>;
        }  // namespace end
    }      // namespace variant_pieces
}  // namespace detail::step

namespace step {
    struct MatchQuantifier : public detail::step::variant_pieces::match_quantifier::Step {};
    struct FinishQuantifier : public detail::step::variant_pieces::finish_quantifier::Step {};
    struct MatchCharClass : public detail::step::variant_pieces::match_char_class::Step {};
    struct MatchLiteral : public detail::step::variant_pieces::match_literal::Step {};
    struct MatchWildcard : public detail::step::variant_pieces::match_wildcard::Step {};
    struct BeginGroup : public detail::step::variant_pieces::begin_group::Step {};
    struct EndGroup : public detail::step::variant_pieces::end_group::Step {};
    struct MatchAlternatives : public detail::step::variant_pieces::match_alternatives::Step {};
    struct FinishAlternatives : public detail::step::variant_pieces::finish_alternatives::Step {};
    struct Backtrack : public detail::step::variant_pieces::backtrack::Step {};
    struct End : public detail::step::variant_pieces::end::Step {};
}  // namespace step

}  // namespace wr22::regex_executor::algorithms::backtracking
