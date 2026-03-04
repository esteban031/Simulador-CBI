#include "parser.h"

#include <array>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {
std::string trim(const std::string& input) {
    std::size_t start = 0;
    while (start < input.size() &&
           std::isspace(static_cast<unsigned char>(input[start])) != 0) {
        ++start;
    }

    std::size_t end = input.size();
    while (end > start &&
           std::isspace(static_cast<unsigned char>(input[end - 1])) != 0) {
        --end;
    }

    return input.substr(start, end - start);
}

bool isCommentLine(const std::string& line) {
    return line.rfind("#", 0) == 0 || line.rfind("//", 0) == 0;
}

bool shouldSkipLine(const std::string& rawLine) {
    const std::string stripped = trim(rawLine);
    return stripped.empty() || isCommentLine(stripped);
}

const std::unordered_set<std::string>& supportedOpcodes() {
    static const std::unordered_set<std::string> kOpcodes = {
        "SET", "LDR", "ADD", "INC", "DEC", "STR", "SHW", "PAUSE", "END"};
    return kOpcodes;
}

[[noreturn]] void throwParseError(
    std::size_t lineNumber, const std::string& rawLine, const std::string& message) {
    throw std::runtime_error(
        "Line " + std::to_string(lineNumber) + ": " + message + ". Raw: \"" + rawLine + "\"");
}

std::array<std::string, 4> parseOperands(
    std::istringstream& iss, std::size_t /*lineNumber*/, const std::string& /*rawLine*/) {
    std::string token;
    std::array<std::string, 4> operands = {"NULL", "NULL", "NULL", "NULL"};
    std::size_t index = 0;
    while (iss >> token) {
        if (index < operands.size()) {
            operands[index] = std::move(token);
            ++index;
        }
    }
    return operands;
}

Instruction parseInstructionLine(const std::string& rawLine, std::size_t lineNumber) {
    const std::string stripped = trim(rawLine);
    std::istringstream iss(stripped);

    std::string opcode;
    iss >> opcode;
    if (supportedOpcodes().find(opcode) == supportedOpcodes().end()) {
        throwParseError(lineNumber, rawLine, "Unknown opcode '" + opcode + "'");
    }

    const std::array<std::string, 4> operands = parseOperands(iss, lineNumber, rawLine);
    return Instruction(opcode, operands, lineNumber, rawLine);
}
}  // namespace

std::vector<Instruction> Parser::parseProgram(const std::string& programPath) const {
    std::ifstream input(programPath);
    if (!input.is_open()) {
        throw std::runtime_error("Could not open program file: " + programPath);
    }

    std::vector<Instruction> instructions;
    std::string line;
    std::size_t lineNumber = 0;
    while (std::getline(input, line)) {
        ++lineNumber;
        if (shouldSkipLine(line)) {
            continue;
        }
        instructions.push_back(parseInstructionLine(line, lineNumber));
    }

    return instructions;
}
