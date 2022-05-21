
// wr22
#include "wr22/regex_explainer/explanation/explanation.hpp"
#include "wr22/unicode/conversion.hpp"

// STL
#include <string_view>

// vector
#include <vector>

// variant
#include <variant>

namespace wr22::regex_explainer::explanation {

std::vector<Explanation> get_full_explanation(const SpannedPart& spanned_part, size_t depth) {
    std::vector<Explanation> result;
    spanned_part.part().visit(
        [&result, depth](const Empty& empty_part) {
            auto sample = get_sample(empty_part);
            result.emplace_back(Explanation(sample, depth));
        },

        [&result, depth](const Literal& part) {
            auto sample = get_sample(part);

            auto literal = std::u32string(1, part.character);

            auto encode_literal = static_cast<uint32_t>(literal[0]);

            result.emplace_back(literal, depth);
            result.emplace_back(sample[0], depth);
            result.emplace_back(literal, depth);
            result.emplace_back(sample[1], depth);
            result.emplace_back(encode_literal, depth);
            result.emplace_back(sample[2], depth);
        },

        [&result, depth](const Alternatives& part) {
            auto sample = get_sample(part);

            int counter = 1;
            for (const auto& alt : part.alternatives) {
                auto current_sample = upgrade_sample(std::to_string(counter), sample);

                result.emplace_back(current_sample, depth, true);

                auto inner_result = get_full_explanation(alt, depth + 1);

                for (auto& inner_result_explanation : inner_result) {
                    result.emplace_back(inner_result_explanation);
                }

                counter++;
            }
        },

        [&result, depth](const Sequence& part) {
            for (const auto& item : part.items) {
                auto inner_result = get_full_explanation(item, depth + 1);

                for (auto& inner_result_explanation : inner_result) {
                    result.emplace_back(inner_result_explanation);
                }
            }
        },

        [&result, depth](const Group& part) {
            auto sample = get_sample(part);
            result.emplace_back(sample, depth, true);

            auto inner_result = get_full_explanation(*part.inner, depth + 1);

            for (auto& inner_result_explanation : inner_result) {
                result.emplace_back(inner_result_explanation);
            }
        },

        [&result, depth](const Optional& part) {
            auto sample = get_sample(part);

            auto inner_result = get_full_explanation(*part.inner, depth);
            for (auto& inner_result_explanation : inner_result) {
                result.emplace_back(inner_result_explanation);
            }

            auto upgraded_sample = upgrade_sample("?", sample);
            result.emplace_back(upgraded_sample, depth);
        },

        [&result, depth](const Plus& part) {
            auto sample = get_sample(part);

            auto inner_result = get_full_explanation(*part.inner, depth);
            for (auto& inner_result_explanation : inner_result) {
                result.emplace_back(inner_result_explanation);
            }

            auto upgraded_sample = upgrade_sample("+", sample);
            result.emplace_back(upgraded_sample, depth);
        },

        [&result, depth](const Star& part) {
            auto sample = get_sample(part);

            auto inner_result = get_full_explanation(*part.inner, depth);
            for (auto& inner_result_explanation : inner_result) {
                result.emplace_back(inner_result_explanation);
            }

            auto upgraded_sample = upgrade_sample("*", sample);
            result.emplace_back(upgraded_sample, depth);
        },

        [&result, depth]([[maybe_unused]] const Wildcard& part) {
            auto sample = get_sample(part);

            auto upgraded_sample = upgrade_sample(".", sample);

            result.emplace_back(upgraded_sample, depth);
        },

        [&result, depth](const CharacterClass& part) {
            auto sample = get_sample(part);

            result.emplace_back(sample[0], depth, true);

            size_t counter = 1;
            for (const auto& spanned_range : part.data.ranges) {

                auto first = std::u32string(1, spanned_range.range.first());
                auto last = std::u32string(1, spanned_range.range.last());

                result.emplace_back(first, depth + counter);
                result.emplace_back("-", depth + counter);
                result.emplace_back(last, depth + counter);
                result.emplace_back(sample[1], depth + counter);
                result.emplace_back(first, depth + counter);
                result.emplace_back(sample[2], depth + counter);
                result.emplace_back(last, depth + counter);
                result.emplace_back(sample[3], depth + counter);

                counter++;
            }

            if (part.data.inverted) {
                result.emplace_back("(inverted)", depth + counter);
            }
        });

    return result;
}

}  // namespace wr22::regex_explainer::explanation
