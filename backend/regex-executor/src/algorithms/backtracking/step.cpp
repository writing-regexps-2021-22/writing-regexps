// wr22
#include <wr22/regex_executor/algorithms/backtracking/step.hpp>
#include <wr22/unicode/conversion.hpp>

namespace wr22::regex_executor::algorithms::backtracking {

#define FIELD_TO_JSON(j, value, field) j[#field] = value.field

namespace {
    template <typename Success, typename Failure, typename SuccessF, typename FailureF>
    void result_to_json(
        nlohmann::json& j,
        const wr22::utils::Adt<Success, Failure>& result,
        const SuccessF& success_func,
        const FailureF& failure_func) {
        result.visit(
            [&j, &success_func](const Success& success) {
                j["success"] = true;
                success_func(success);
            },
            [&j, &failure_func](const Failure& failure) {
                j["success"] = false;
                failure_func(failure);
            });
    }
}  // namespace

namespace step {
    const char* MatchQuantifier::type_code() const {
        switch (quantifier_type) {
        case QuantifierType::Star:
            return "match_star";
        case QuantifierType::Plus:
            return "match_plus";
        case QuantifierType::Optional:
            return "match_optional";
        }
    }

    void to_json(nlohmann::json& j, const MatchQuantifier& step) {
        j = nlohmann::json::object();
        FIELD_TO_JSON(j, step, regex_span);
        FIELD_TO_JSON(j, step, string_pos);
    }

    const char* FinishQuantifier::type_code() const {
        switch (quantifier_type) {
        case QuantifierType::Star:
            return "finish_star";
        case QuantifierType::Plus:
            return "finish_plus";
        case QuantifierType::Optional:
            return "finish_optional";
        }
    }

    void to_json(nlohmann::json& j, const FinishQuantifier& step) {
        j = nlohmann::json::object();
        FIELD_TO_JSON(j, step, regex_span);
        result_to_json(
            j,
            step.result,
            [&](const auto& success) {
                FIELD_TO_JSON(j, success, string_span);
                FIELD_TO_JSON(j, success, num_repetitions);
            },
            [&](const auto& failure) {
                FIELD_TO_JSON(j, failure, string_pos);
                FIELD_TO_JSON(j, failure, failure_reason);
            });
    }

    void to_json(nlohmann::json& j, const MatchCharClass& step) {
        FIELD_TO_JSON(j, step, regex_span);
        result_to_json(
            j,
            step.result,
            [&](const auto& success) { FIELD_TO_JSON(j, success, string_span); },
            [&](const auto& failure) {
                FIELD_TO_JSON(j, failure, string_pos);
                FIELD_TO_JSON(j, failure, failure_reason);
            });
    }

    void to_json(nlohmann::json& j, const MatchWildcard& step) {
        FIELD_TO_JSON(j, step, regex_span);
        result_to_json(
            j,
            step.result,
            [&](const auto& success) { FIELD_TO_JSON(j, success, string_span); },
            [&](const auto& failure) {
                FIELD_TO_JSON(j, failure, string_pos);
                FIELD_TO_JSON(j, failure, failure_reason);
            });
    }

    void to_json(nlohmann::json& j, const BeginGroup& step) {
        FIELD_TO_JSON(j, step, regex_span);
        FIELD_TO_JSON(j, step, string_pos);
    }

    void to_json(nlohmann::json& j, const EndGroup& step) {
        FIELD_TO_JSON(j, step, string_pos);
    }

    void to_json(nlohmann::json& j, const MatchLiteral& step) {
        FIELD_TO_JSON(j, step, regex_span);
        j["literal"] = wr22::unicode::to_utf8(step.literal);
        result_to_json(
            j,
            step.result,
            [&](const auto& success) { FIELD_TO_JSON(j, success, string_span); },
            [&](const auto& failure) {
                FIELD_TO_JSON(j, failure, string_pos);
                FIELD_TO_JSON(j, failure, failure_reason);
            });
    }

    void to_json(nlohmann::json& j, const MatchAlternatives& step) {
        FIELD_TO_JSON(j, step, regex_span);
        FIELD_TO_JSON(j, step, string_pos);
    }

    void to_json(nlohmann::json& j, const FinishAlternatives& step) {
        FIELD_TO_JSON(j, step, regex_span);
        result_to_json(
            j,
            step.result,
            [&](const auto& success) {
                FIELD_TO_JSON(j, success, string_span);
                FIELD_TO_JSON(j, success, alternative_chosen);
            },
            [&](const auto& failure) {
                FIELD_TO_JSON(j, failure, string_pos);
                FIELD_TO_JSON(j, failure, failure_reason);
            });
    }

    void to_json(nlohmann::json& j, const Backtrack& step) {
        FIELD_TO_JSON(j, step, string_pos);
        FIELD_TO_JSON(j, step, continue_after_step);
    }

    void to_json(nlohmann::json& j, const End& step) {
        FIELD_TO_JSON(j, step, string_pos);
        result_to_json(
            j,
            step.result,
            [](const auto&) {},
            [](const auto&) {});
    }
}  // namespace step

void to_json(nlohmann::json& j, const Step& step) {
    step.visit([&j](const auto& specific_step) {
        j = specific_step;
        j["type"] = specific_step.type_code();
    });
}

}  // namespace wr22::regex_executor::algorithms::backtracking
