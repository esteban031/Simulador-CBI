#pragma once

#include <string>
#include <vector>

#include "instruction.h"

class Parser {
public:
    std::vector<Instruction> parseProgram(const std::string& programPath) const;
};