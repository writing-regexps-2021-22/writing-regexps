// wr22
#include <stdexcept>
#include <wr22/regex_parser/parser/regex.hpp>
#include <wr22/regex_parser/regex/part.hpp>
#include <wr22/regex_parser/utils/utf8_string_view.hpp>

// boost
#include <boost/spirit/include/qi.hpp>

// stl
#include <string>
#include <vector>

namespace wr22::regex_parser::parser {

namespace qi = boost::spirit::qi;

template <typename Iterator>
struct Grammar : public boost::spirit::qi::grammar<Iterator, regex::Part()> {
    Grammar() : Grammar::base_type(regex) {
        using qi::_1;
        using qi::_val;

        regex = alternatives;
        alternatives = (sequence % U'|')[([](auto&& vec) {
            return regex::part::Alternatives{std::forward<decltype(vec)>(vec)};
        })];
        sequence = (*atom)[(
            [](auto&& vec) { return regex::part::Sequence{std::forward<decltype(vec)>(vec)}; })];
        atom = char_literal[([](auto lit) { return regex::part::Literal{lit}; })] | group;
        char_literal = qi::char_;
        group =
            ((qi::lit(U"(?P<") >> group_name >> U'>' >> regex >> U')')
             | (qi::lit(U"(?<") >> group_name >> U'>' >> regex >> U')')
             | (qi::lit(U"(?'") >> group_name >> U'\'' >> regex
                >> U')'))[([](auto&& tuple) { throw "Not implemented"; })];
        group_name = +(qi::char_ - qi::char_(U"'<>()"));
    }

    qi::rule<Iterator, regex::Part()> regex;
    qi::rule<Iterator, regex::Part(std::vector<regex::Part>)> alternatives;
    qi::rule<Iterator, regex::Part()> sequence;
    qi::rule<Iterator, regex::Part()> atom;
    qi::rule<Iterator, regex::Part(char32_t)> char_literal;
    qi::rule<Iterator, regex::Part()> group;
    qi::rule<Iterator, std::vector<char32_t>()> group_name;
};

regex::Part parse_regex(const utils::UnicodeStringView& regex) {
    auto grammar = Grammar<utils::UnicodeStringViewIterator>();
    regex::Part result = regex::part::Empty{};
    bool ok = parse(regex.begin(), regex.end(), grammar, result);
    if (!ok) {
        throw std::runtime_error("Parser returned error");
    }
    return result;
}

}  // namespace wr22::regex_parser::parser
