#pragma once

#include <cstddef>
#include <string>
#include <vector>

class Memory {
public:
    explicit Memory(std::size_t size = 256);

    bool validateAddress(const std::string& dn) const;
    long long read(std::size_t address) const;
    void write(std::size_t address, long long value);

private:
    bool isValidIndex(std::size_t address) const;
    std::vector<long long> cells_;
};
