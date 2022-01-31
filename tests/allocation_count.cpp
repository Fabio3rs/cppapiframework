#include "allocation_count.hpp"
#include <cstddef>
#include <cstdlib>

static std::atomic<std::size_t> allocations{0};
static std::atomic<std::size_t> deallocations{0};

auto operator new(std::size_t n) -> void* {
    ++allocations;
    return malloc(n); // NOLINT(hicpp-no-malloc)
}

void operator delete(void *p) noexcept {
    ++deallocations;
    free(p); // NOLINT(hicpp-no-malloc)
}

auto AllocationCount::getAllocationCount() -> std::atomic<std::size_t>&{
    return allocations;
}

auto AllocationCount::getDeallocationCount() -> std::atomic<std::size_t>&{
    return deallocations;
}


