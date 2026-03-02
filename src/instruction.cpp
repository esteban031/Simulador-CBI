#include "instruction.h"

#include <utility>

Instruction::Instruction()
    : opcode(),
      operands{{"NULL", "NULL", "NULL", "NULL"}},
      lineNumber(0),
      rawLine() {}

Instruction::Instruction(
    std::string opcode,
    std::array<std::string, 4> operands,
    std::size_t lineNumber,
    std::string rawLine)
    : opcode(std::move(opcode)),
      operands(std::move(operands)),
      lineNumber(lineNumber),
      rawLine(std::move(rawLine)) {}
