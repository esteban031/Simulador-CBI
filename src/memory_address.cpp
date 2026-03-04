#include "memory_address.h"

MemoryAddress::MemoryAddress(Memory& memory, std::size_t address)
    : memory_(&memory), constMemory_(&memory), address_(address) {}

MemoryAddress::MemoryAddress(const Memory& memory, std::size_t address)
    : memory_(nullptr), constMemory_(&memory), address_(address) {}

long long MemoryAddress::read() const {
    return constMemory_->read(address_);
}

void MemoryAddress::write(long long value) {
    if (memory_ == nullptr) {
        return;
    }
    memory_->write(address_, value);
}

std::size_t MemoryAddress::index() const {
    return address_;
}
