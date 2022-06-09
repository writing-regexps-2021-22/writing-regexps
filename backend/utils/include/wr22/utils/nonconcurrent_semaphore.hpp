#pragma once

// stl
#include <cstddef>
#include <functional>
#include <optional>

namespace wr22::utils {

class NonconcurrentSemaphoreGuard;

class NonconcurrentSemaphore {
public:
    explicit NonconcurrentSemaphore(size_t max_capacity);

    std::optional<NonconcurrentSemaphoreGuard> enter();

private:
    bool raw_enter();
    void raw_exit();

    size_t m_occupied;
    size_t m_max_capacity;

    friend class NonconcurrentSemaphoreGuard;
};

class NonconcurrentSemaphoreGuard {
public:
    NonconcurrentSemaphoreGuard(const NonconcurrentSemaphoreGuard&) = delete;
    // TODO: undelete the move constructor if necessary.
    NonconcurrentSemaphoreGuard(NonconcurrentSemaphoreGuard&& other);

    NonconcurrentSemaphoreGuard& operator=(const NonconcurrentSemaphoreGuard&) = delete;
    NonconcurrentSemaphoreGuard& operator=(NonconcurrentSemaphoreGuard&& other);

    ~NonconcurrentSemaphoreGuard();

private:
    explicit NonconcurrentSemaphoreGuard(NonconcurrentSemaphore& semaphore_ref);

    void destroy();

    std::reference_wrapper<NonconcurrentSemaphore> m_semaphore_ref;
    bool m_is_active;

    friend class NonconcurrentSemaphore;
};

}  // namespace wr22::utils
