#pragma once

#include <cstddef>
#include <string>
#include <vector>

class Memory {
public:
    static constexpr std::size_t kMinAddress = 0;
    static constexpr std::size_t kMaxAddress = 39;
    static constexpr std::size_t kMainMemorySize = kMaxAddress + 1;

    Memory();

    bool validateAddress(const std::string& dn) const;
    long long read(std::size_t address) const;
    void write(std::size_t address, long long value);

private:
    bool isValidIndex(std::size_t address) const;
    std::vector<long long> cells_;
};
