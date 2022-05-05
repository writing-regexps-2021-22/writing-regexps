#pragma once

// wr22
#include <wr22/regex_parser/regex/part.hpp>

namespace wr22::regex_executor {

class Regex {
public:
    explicit Regex(regex_parser::regex::SpannedPart root_part);

    const regex_parser::regex::SpannedPart& root_part() const;

private:
    regex_parser::regex::SpannedPart m_root_part;
};

}
