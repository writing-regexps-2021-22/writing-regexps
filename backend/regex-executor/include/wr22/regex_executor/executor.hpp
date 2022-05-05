#pragma once

// wr22
#include <wr22/regex_executor/regex.hpp>

// stl
#include <functional>

namespace wr22::regex_executor {

class Executor {
public:
    explicit Executor(const Regex& regex_ref);

    const Regex& regex_ref() const;

private:
    std::reference_wrapper<const Regex> m_regex_ref;
};

}
