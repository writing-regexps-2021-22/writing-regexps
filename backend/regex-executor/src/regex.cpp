// wr22
#include <wr22/regex_executor/regex.hpp>

namespace wr22::regex_executor {

Regex::Regex(regex_parser::regex::SpannedPart root_part) : m_root_part(std::move(root_part)) {}

const regex_parser::regex::SpannedPart& Regex::root_part() const {
    return m_root_part;
}

}  // namespace wr22::regex_executor
