#pragma once

#include <string>

class Register {
public:
    explicit Register(std::string name, long long initialValue = 0);

    const std::string& getName() const;
    long long get() const;
    void set(long long value);

private:
    std::string name_;
    long long value_;
};
