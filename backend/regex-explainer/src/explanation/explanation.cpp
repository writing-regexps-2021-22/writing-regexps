
// wr22
#include "wr22/regex_explainer/explanation/explanation.hpp"
#include "wr22/unicode/conversion.hpp"

// STL
#include <string_view>

// vector
#include <vector>

// variant
#include <variant>

// fmt
#include <fmt/format.h>

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

            auto literal = wr22::unicode::to_utf8(part.character);

            auto encode_literal = static_cast<uint32_t>(part.character);

            std::string str_literal = fmt::format(
                "{} {} {} {} {} {}",
                literal,
                sample[0],
                literal,
                sample[1],
                encode_literal,
                sample[2]);

            result.emplace_back(str_literal, depth);
        },

        [&result, depth](const Alternatives& part) {
            auto sample = get_sample(part);

            int counter = 1;
            for (const auto& alt : part.alternatives) {
                auto current_sample = upgrade_sample(std::to_string(counter), sample);

                result.emplace_back(current_sample, depth, true);

                auto inner_result = get_full_explanation(alt, depth + 1);
                for (auto&& inner_result_explanation : inner_result) {
                    result.emplace_back(std::move(inner_result_explanation));
                }

                counter++;
            }
        },

        [&result, depth](const Sequence& part) {
            for (const auto& item : part.items) {
                auto inner_result = get_full_explanation(item, depth);
                for (auto&& inner_result_explanation : inner_result) {
                    result.emplace_back(std::move(inner_result_explanation));
                }
            }
        },

        [&result, depth](const Group& part) {
            auto sample = get_sample(part);

            std::optional<std::string> name;
            size_t index = part.capture.visit(
                [](const regex_parser::regex::capture::Index& capture_type) { return 0; },
                [](const regex_parser::regex::capture::None& capture_type) { return 1; },
                [&name](const regex_parser::regex::capture::Name& capture_type) {
                    name = capture_type.name;
                    return 2;
                });

            if (name.has_value()) {
                sample[index] += " " + name.value();
            }
            
            result.emplace_back(sample[index], depth, true);

            auto inner_result = get_full_explanation(*part.inner, depth + 1);
            for (auto&& inner_result_explanation : inner_result) {
                result.emplace_back(std::move(inner_result_explanation));
            }
        },

        [&result, depth](const Optional& part) {
            auto sample = get_sample(part);

            auto inner_result = get_full_explanation(*part.inner, depth);
            for (auto&& inner_result_explanation : inner_result) {
                result.emplace_back(std::move(inner_result_explanation));
            }

            auto upgraded_sample = upgrade_sample("?", sample);
            result.emplace_back(upgraded_sample, depth);
        },

        [&result, depth](const Plus& part) {
            auto sample = get_sample(part);

            auto inner_result = get_full_explanation(*part.inner, depth);
            for (auto&& inner_result_explanation : inner_result) {
                result.emplace_back(std::move(inner_result_explanation));
            }

            auto upgraded_sample = upgrade_sample("+", sample);
            result.emplace_back(upgraded_sample, depth);
        },

        [&result, depth](const Star& part) {
            auto sample = get_sample(part);

            auto inner_result = get_full_explanation(*part.inner, depth);
            for (auto&& inner_result_explanation : inner_result) {
                result.emplace_back(std::move(inner_result_explanation));
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

            for (const auto& spanned_range : part.data.ranges) {

                if (spanned_range.range.is_single_character()) {
                    auto literal = Literal(spanned_range.range.first());
                    SpannedPart literal_spanned_part = SpannedPart(literal, spanned_range.span);

                    auto inner_result = get_full_explanation(literal_spanned_part, depth);
                    for (auto&& inner_result_explanation : inner_result) {
                        result.emplace_back(std::move(inner_result_explanation));
                    }
                } else {
                    auto first = wr22::unicode::to_utf8(spanned_range.range.first());
                    auto last = wr22::unicode::to_utf8(spanned_range.range.last());

                    std::string str_character_class = fmt::format(
                        "{}-{} {} {} {} {} {}",
                        first,
                        last,
                        sample[1],
                        first,
                        sample[2],
                        last,
                        sample[3]);

                    result.emplace_back(str_character_class, depth);
                }
            }

            if (part.data.inverted) {
                result.emplace_back("(inverted)", depth);
            }
        });

    return result;
}

}  // namespace wr22::regex_explainer::explanation
