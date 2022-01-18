// wr22
#include <wr22/regex_parser/parser/regex.hpp>

// peglib
#include <peglib.h>

// cmrc
#include <cmrc/cmrc.hpp>

#include <iostream>

CMRC_DECLARE(grammar);

namespace wr22::regex_parser::parser {

namespace {
    std::string get_grammar() {
        auto fs = cmrc::grammar::get_filesystem();
        auto grammar_file = fs.open("src/parser/regex.peg");
        auto grammar = std::string(grammar_file.begin(), grammar_file.end());
        std::cerr << '[' << grammar << ']' << std::endl;
        return grammar;
    }
}

regex::Part parse_regex(const std::string_view& regex) {
    static auto parser = peg::parser(get_grammar());
    throw 55;
    if (!parser) {
        throw std::runtime_error("Invalid grammar");
    }
}

}  // namespace wr22::regex_parser::parser
