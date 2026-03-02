#pragma once

#include <array>
#include <cstddef>
#include <string>

class Instruction {
public:
    std::string opcode;
    std::array<std::string, 4> operands;
    std::size_t lineNumber;
    std::string rawLine;

    Instruction();
    Instruction(
        std::string opcode,
        std::array<std::string, 4> operands,
        std::size_t lineNumber,
        std::string rawLine);
};
