// wr22
#include <wr22/regex_executor/algorithms/backtracking/decision.hpp>
#include <wr22/regex_executor/algorithms/backtracking/interpreter.hpp>
#include <wr22/regex_executor/algorithms/backtracking/part_executor.hpp>
#include <wr22/regex_executor/algorithms/backtracking/specific_part_executor.hpp>
#include <wr22/regex_executor/utils/spanned_ref.hpp>
#include <wr22/regex_parser/regex/part.hpp>

// stl
#include <type_traits>

namespace wr22::regex_executor::algorithms::backtracking {

namespace {
    using regex_parser::regex::CharacterClassData;
    bool char_class_matches(const CharacterClassData& data, char32_t c) {
        bool range_matched = false;
        for (const auto& range : data.ranges) {
            if (range.range.contains(c)) {
                range_matched = true;
                break;
            }
        }
        return range_matched ^ data.inverted;
    }

    template <typename T>
    struct is_decision_making : public std::false_type {};

    template <>
    struct is_decision_making<regex_parser::regex::part::Alternatives> : public std::true_type {};
    template <>
    struct is_decision_making<regex_parser::regex::part::Star> : public std::true_type {};
    template <>
    struct is_decision_making<regex_parser::regex::part::Plus> : public std::true_type {};
    template <>
    struct is_decision_making<regex_parser::regex::part::Optional> : public std::true_type {};

    template <typename T>
    constexpr bool is_decision_making_v = is_decision_making<T>::value;
}  // namespace

PartExecutor::PartExecutor(utils::SpannedRef<regex_parser::regex::Part> part_ref)
    : m_part_ref(part_ref) {}

bool PartExecutor::execute(Interpreter& interpreter) const {
    return m_part_ref.item().visit([&](const auto& part) {
        auto ref = utils::SpannedRef(part, m_part_ref.span());
        using PartT = std::remove_cvref_t<decltype(part)>;
        auto executor = [&]() {
            if constexpr (is_decision_making_v<PartT>) {
                return SpecificPartExecutor<PartT>(ref, m_part_ref);
            } else {
                return SpecificPartExecutor<PartT>(ref);
            }
        }();
        return executor.execute(interpreter);
    });
}

}  // namespace wr22::regex_executor::algorithms::backtracking
