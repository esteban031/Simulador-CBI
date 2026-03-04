#include "memory.h"

#include <cctype>

Memory::Memory() : cells_(kMainMemorySize, 0) {}

bool Memory::validateAddress(const std::string& dn) const {
    if (dn.size() < 2) {
        return false;
    }

    const char prefix = dn[0];
    if (prefix != 'D' && prefix != 'd') {
        return false;
    }

    std::size_t numericPart = 0;
    for (std::size_t i = 1; i < dn.size(); ++i) {
        const unsigned char ch = static_cast<unsigned char>(dn[i]);
        if (std::isdigit(ch) == 0) {
            return false;
        }
        numericPart = (numericPart * 10) + static_cast<std::size_t>(dn[i] - '0');
    }

    return isValidIndex(numericPart);
}

bool Memory::isValidIndex(std::size_t address) const {
    return address < kMainMemorySize;
}

long long Memory::read(std::size_t address) const {
    if (!isValidIndex(address)) {
        return 0;
    }
    return cells_[address];
}

void Memory::write(std::size_t address, long long value) {
    if (!isValidIndex(address)) {
        return;
    }
    cells_[address] = value;
}
