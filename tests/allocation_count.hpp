#pragma once
#include <atomic>
#include <cstdint>

class AllocationCount{

public:
    static auto getAllocationCount() -> std::atomic<std::size_t>&;
    static auto getDeallocationCount() -> std::atomic<std::size_t>&;

};
