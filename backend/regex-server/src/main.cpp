#include <wr22/regex_parser/parser/regex.hpp>
#include <iostream>

int main() {
    auto regex = U"a(b|c)+";
    std::cout << wr22::regex_parser::parser::parse_regex(regex) << std::endl;
}
