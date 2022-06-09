// wr22
#include <wr22/utils/nonconcurrent_semaphore.hpp>

// stl
#include <stdexcept>

namespace wr22::utils {

NonconcurrentSemaphore::NonconcurrentSemaphore(size_t max_capacity)
    : m_occupied(0), m_max_capacity(max_capacity) {}

std::optional<NonconcurrentSemaphoreGuard> NonconcurrentSemaphore::enter() {
    auto ok = raw_enter();
    if (!ok) {
        return std::nullopt;
    }

    return NonconcurrentSemaphoreGuard(*this);
}

bool NonconcurrentSemaphore::raw_enter() {
    if (m_occupied >= m_max_capacity) {
        return false;
    }
    ++m_occupied;
    return true;
}

void NonconcurrentSemaphore::raw_exit() {
    if (m_occupied == 0) {
        throw std::logic_error("NonconcurrentSemaphore exited while empty");
    }
    --m_occupied;
}

NonconcurrentSemaphoreGuard::NonconcurrentSemaphoreGuard(NonconcurrentSemaphoreGuard&& other)
    : m_semaphore_ref(other.m_semaphore_ref), m_is_active(other.m_is_active) {
    other.m_is_active = false;
}

NonconcurrentSemaphoreGuard& NonconcurrentSemaphoreGuard::operator=(
    NonconcurrentSemaphoreGuard&& other) {
    if (m_is_active) {
        destroy();
    }
    m_semaphore_ref = other.m_semaphore_ref;
    m_is_active = other.m_is_active;
    other.m_is_active = false;
    return *this;
}

NonconcurrentSemaphoreGuard::~NonconcurrentSemaphoreGuard() {
    destroy();
}

void NonconcurrentSemaphoreGuard::destroy() {
    if (m_is_active) {
        m_semaphore_ref.get().raw_exit();
    }
}

NonconcurrentSemaphoreGuard::NonconcurrentSemaphoreGuard(NonconcurrentSemaphore& semaphore_ref)
    : m_semaphore_ref(semaphore_ref), m_is_active(true) {}

}  // namespace wr22::utils
