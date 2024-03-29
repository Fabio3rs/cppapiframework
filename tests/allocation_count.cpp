#include "allocation_count.hpp"
#include <cstddef>
#include <cstdlib>

void operator delete(void *ptr, size_t blksize) noexcept;

static std::atomic<std::size_t> allocations{0};
static std::atomic<std::size_t> allocationSize{0};
static std::atomic<std::size_t> deallocations{0};

auto operator new(std::size_t n) -> void * {
    ++allocations;
    allocationSize += n;
    return malloc(n); // NOLINT(hicpp-no-malloc)
}

void operator delete(void *ptr) noexcept {
    ++deallocations;
    free(ptr); // NOLINT(hicpp-no-malloc)
}

void operator delete(void *ptr, size_t /*blksize*/) noexcept {
    ++deallocations;
    free(ptr); // NOLINT(hicpp-no-malloc)
}

auto AllocationCount::getAllocationCount() -> std::atomic<std::size_t> & {
    return allocations;
}

auto AllocationCount::getAllocationSize() -> std::atomic<std::size_t> & {
    return allocationSize;
}

auto AllocationCount::getDeallocationCount() -> std::atomic<std::size_t> & {
    return deallocations;
}

auto AllocationCount::optimizationBarrier(void *ptr) -> void { (void)ptr; }
