#pragma once

#include <string>

#include "cpu.h"
#include "parser.h"

class Simulator {
public:
    explicit Simulator(std::string programPath, bool verbose = false);

    int run();

private:
    std::string programPath_;
    bool verbose_;
    Parser parser_;
    Cpu cpu_;
};
