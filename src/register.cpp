#include "register.h"

#include <utility>

Register::Register(std::string name, long long initialValue)
    : name_(std::move(name)), value_(initialValue) {}

const std::string& Register::getName() const {
    return name_;
}

long long Register::get() const {
    return value_;
}

void Register::set(long long value) {
    value_ = value;
}
