#pragma once

#include "wr22/regex_explainer/explanation/upgrade_sample.hpp"

namespace wr22::regex_explainer::explanation {

    std::string_view upgrade_sample_altertative(char symbol, const std::string_view &sample) {
        char upgraded_sample[alternative_sz];

        upgraded_sample[0] = symbol;

        for (size_t i = 0; i < sample.size(); ++i) {
            upgraded_sample[i + 1] = sample[i];
        }

        auto result = std::string_view(
                upgraded_sample,
                alternative_sz
        );

        return result;
    }

    std::string_view upgrade_sample_optional(char symbol, const std::string_view &sample) {
        char upgraded_sample[optional_sz];

        upgraded_sample[0] = symbol;

        for (size_t i = 0; i < sample.size(); ++i) {
            upgraded_sample[i + 1] = sample[i];
        }

        auto result = std::string_view(
                upgraded_sample,
                optional_sz
        );

        return result;
    }

    std::string_view upgrade_sample_plus(char symbol, const std::string_view &sample) {
        char upgraded_sample[plus_sz];

        upgraded_sample[0] = symbol;

        for (size_t i = 0; i < sample.size(); ++i) {
            upgraded_sample[i + 1] = sample[i];
        }

        auto result = std::string_view(
                upgraded_sample,
                plus_sz
        );

        return result;
    }

    std::string_view upgrade_sample_star(char symbol, const std::string_view &sample) {
        char upgraded_sample[star_sz];

        upgraded_sample[0] = symbol;

        for (size_t i = 0; i < sample.size(); ++i) {
            upgraded_sample[i + 1] = sample[i];
        }

        auto result = std::string_view(
                upgraded_sample,
                star_sz
        );

        return result;
    }

    std::string_view upgrade_sample_wildcard(char symbol, const std::string_view &sample) {
        char upgraded_sample[wildcard_sz];

        upgraded_sample[0] = symbol;

        for (size_t i = 0; i < sample.size(); ++i) {
            upgraded_sample[i + 1] = sample[i];
        }

        auto result = std::string_view(
                upgraded_sample,
                wildcard_sz
        );

        return result;
    }

} // wr22::regex_explainer::explanation

