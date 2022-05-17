// catch
#include <catch2/catch.hpp>

// wr22
#include <wr22/regex_executor/executor.hpp>
#include <wr22/regex_executor/regex.hpp>
#include <wr22/regex_parser/parser/regex.hpp>

// stl
#include <iostream>

using wr22::regex_executor::Executor;
using wr22::regex_executor::Regex;
using wr22::regex_parser::parser::parse_regex;

TEST_CASE("Sequences are executed correctly") {
    //auto regex = Regex(parse_regex(U"gr[ae]+y"));
    auto regex = Regex(parse_regex(U"(.*)ll"));
    auto ex = Executor(regex);
    //auto res = ex.execute(U"graey");
    auto res = ex.execute(U"ball");
    std::cout << nlohmann::json(res.steps).dump(4) << std::endl;
}
