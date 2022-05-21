// wr22
#include <wr22/utils/box.hpp>

namespace wr22::utils {

const char* BoxIsEmpty::what() const noexcept {
    return "A Box does not contain a value, so it cannot be dereferenced.";
}

}  // namespace wr22::utils
