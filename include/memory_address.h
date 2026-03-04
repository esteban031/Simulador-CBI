#pragma once

#include <cstddef>

#include "memory.h"

class MemoryAddress {
public:
    MemoryAddress(Memory& memory, std::size_t address);
    MemoryAddress(const Memory& memory, std::size_t address);

    long long read() const;
    void write(long long value);
    std::size_t index() const;

private:
    Memory* memory_;
    const Memory* constMemory_;
    std::size_t address_;
};
