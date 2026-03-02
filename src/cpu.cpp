#include "cpu.h"

#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

Cpu::Cpu()
    : program_(),
      memory_(),
      acc_("ACC"),
      icr_("ICR"),
      mar_("MAR"),
      mdr_("MDR"),
      ucState_(UcState::HALT),
      verbose_(false) {}

void Cpu::loadProgram(const std::vector<Instruction>& program) {
    program_ = program;
    acc_.set(0);
    icr_.set(0);
    mar_.set(0);
    mdr_.set(0);
    ucState_ = UcState::HALT;
}

void Cpu::setVerbose(bool verbose) {
    verbose_ = verbose;
}

void Cpu::run() {
    if (program_.empty()) {
        ucState_ = UcState::HALT;
        return;
    }

    ucState_ = UcState::FETCH;
    while (ucState_ != UcState::HALT) {
        const Instruction* currentInstruction = fetchNextInstruction();
        if (currentInstruction == nullptr) {
            break;
        }

        ucState_ = UcState::DECODE;
        decodeInstruction(*currentInstruction);
        logDecode(*currentInstruction);

        ucState_ = UcState::EXECUTE;
        executeInstruction(*currentInstruction);
        logExecute();
    }
}

void Cpu::throwInstructionError(
    const Instruction& instruction, const std::string& message) const {
    throw std::runtime_error(
        "Line " + std::to_string(instruction.lineNumber) + ": " + message + ". Raw: \"" +
        instruction.rawLine + "\"");
}

bool Cpu::isNullToken(const std::string& token) const {
    return token == "NULL";
}

bool Cpu::isRegisterToken(const std::string& token) const {
    return token == "ACC" || token == "ICR" || token == "MAR" || token == "MDR" ||
           token == "UC";
}

const Instruction* Cpu::fetchNextInstruction() {
    ucState_ = UcState::FETCH;

    const long long currentIcr = icr_.get();
    if (currentIcr < 0) {
        throw std::runtime_error("ICR cannot be negative");
    }

    const std::size_t index = static_cast<std::size_t>(currentIcr);
    if (index >= program_.size()) {
        ucState_ = UcState::HALT;
        return nullptr;
    }

    const Instruction& instruction = program_[index];
    logFetch(instruction, currentIcr);
    icr_.set(currentIcr + 1);
    return &instruction;
}

void Cpu::logFetch(const Instruction& instruction, long long currentIcr) const {
    if (!verbose_) {
        return;
    }

    std::cerr << "[FETCH] ICR=" << currentIcr << " RAW=\"" << instruction.rawLine << "\"\n";
}

void Cpu::logDecode(const Instruction& instruction) const {
    if (!verbose_) {
        return;
    }

    std::cerr << "[DECODE] opcode=" << instruction.opcode << " ACC=" << acc_.get()
              << " ICR=" << icr_.get() << " MAR=" << mar_.get() << " MDR=" << mdr_.get()
              << '\n';
}

void Cpu::logExecute() const {
    if (!verbose_) {
        return;
    }

    std::cerr << "[EXECUTE] ACC=" << acc_.get() << " ICR=" << icr_.get()
              << " MAR=" << mar_.get() << " MDR=" << mdr_.get()
              << " UC=" << ucStateToString(ucState_) << '\n';
}

std::size_t Cpu::parseAddressToken(
    const Instruction& instruction, const std::string& token) const {
    if (!memory_.validateAddress(token)) {
        throwInstructionError(instruction, "Invalid address token '" + token + "'");
    }

    return static_cast<std::size_t>(std::stoull(token.substr(1)));
}

long long Cpu::parseImmediateToken(const Instruction& instruction, const std::string& token) {
    if (token.empty() || isNullToken(token)) {
        throwInstructionError(instruction, "Missing immediate value");
    }

    if (!token.empty() && token[0] == 'D') {
        return readMemory(instruction, token);
    }

    std::size_t consumedChars = 0;
    long long value = 0;
    try {
        value = std::stoll(token, &consumedChars, 10);
    } catch (const std::exception&) {
        throwInstructionError(instruction, "Invalid immediate value '" + token + "'");
    }

    if (consumedChars != token.size()) {
        throwInstructionError(instruction, "Invalid immediate value '" + token + "'");
    }

    return value;
}

long long Cpu::readMemory(const Instruction& instruction, const std::string& addressToken) {
    const std::size_t address = parseAddressToken(instruction, addressToken);
    const long long value = memory_.read(address);
    mar_.set(static_cast<long long>(address));
    mdr_.set(value);
    return value;
}

void Cpu::writeMemory(
    const Instruction& instruction, const std::string& addressToken, long long value) {
    const std::size_t address = parseAddressToken(instruction, addressToken);
    memory_.write(address, value);
    mar_.set(static_cast<long long>(address));
    mdr_.set(value);
}

void Cpu::decodeInstruction(const Instruction& instruction) const {
    const std::string& opcode = instruction.opcode;

    if (opcode == "SET") {
        decodeSet(instruction);
        return;
    }

    if (opcode == "LDR" || opcode == "INC" || opcode == "DEC" || opcode == "STR") {
        decodeSingleAddress(instruction, opcode);
        return;
    }

    if (opcode == "ADD") {
        decodeAdd(instruction);
        return;
    }

    if (opcode == "SHW") {
        decodeShw(instruction);
        return;
    }

    if (opcode == "PAUSE" || opcode == "END") {
        return;
    }

    throwInstructionError(instruction, "Unsupported opcode '" + opcode + "'");
}

void Cpu::decodeSet(const Instruction& instruction) const {
    const std::array<std::string, 4>& operands = instruction.operands;
    if (isNullToken(operands[0]) || isNullToken(operands[1])) {
        throwInstructionError(instruction, "SET requires destination and value");
    }
}

void Cpu::decodeSingleAddress(const Instruction& instruction, const std::string& opcode) const {
    if (isNullToken(instruction.operands[0])) {
        throwInstructionError(instruction, opcode + " requires an address");
    }
}

void Cpu::decodeAdd(const Instruction& instruction) const {
    if (isNullToken(instruction.operands[0])) {
        throwInstructionError(instruction, "ADD requires at least one address");
    }
}

void Cpu::decodeShw(const Instruction& instruction) const {
    if (isNullToken(instruction.operands[0])) {
        throwInstructionError(instruction, "SHW requires one operand");
    }
}

void Cpu::executeInstruction(const Instruction& instruction) {
    const std::string& opcode = instruction.opcode;

    if (opcode == "SET") {
        executeSet(instruction);
        return;
    }

    if (opcode == "LDR") {
        executeLdr(instruction);
        return;
    }

    if (opcode == "ADD") {
        executeAdd(instruction);
        return;
    }

    if (opcode == "INC") {
        executeInc(instruction);
        return;
    }

    if (opcode == "DEC") {
        executeDec(instruction);
        return;
    }

    if (opcode == "STR") {
        executeStr(instruction);
        return;
    }

    if (opcode == "SHW") {
        executeShw(instruction);
        return;
    }

    if (opcode == "PAUSE") {
        executePause();
        return;
    }

    if (opcode == "END") {
        executeEnd();
        return;
    }

    throwInstructionError(instruction, "Unsupported opcode '" + opcode + "'");
}

void Cpu::executeSet(const Instruction& instruction) {
    const std::array<std::string, 4>& operands = instruction.operands;
    const long long value = parseImmediateToken(instruction, operands[1]);
    writeMemory(instruction, operands[0], value);
}

void Cpu::executeLdr(const Instruction& instruction) {
    acc_.set(readMemory(instruction, instruction.operands[0]));
}

void Cpu::executeAdd(const Instruction& instruction) {
    const std::array<std::string, 4>& operands = instruction.operands;
    if (isNullToken(operands[1])) {
        const long long value = readMemory(instruction, operands[0]);
        acc_.set(acc_.get() + value);
        return;
    }

    const long long lhs = readMemory(instruction, operands[0]);
    const long long rhs = readMemory(instruction, operands[1]);
    const long long result = lhs + rhs;
    acc_.set(result);

    if (!isNullToken(operands[2])) {
        writeMemory(instruction, operands[2], result);
    }
}

void Cpu::executeInc(const Instruction& instruction) {
    const std::string& destination = instruction.operands[0];
    const long long value = readMemory(instruction, destination) + 1;
    writeMemory(instruction, destination, value);
}

void Cpu::executeDec(const Instruction& instruction) {
    const std::string& destination = instruction.operands[0];
    const long long value = readMemory(instruction, destination) - 1;
    writeMemory(instruction, destination, value);
}

void Cpu::executeStr(const Instruction& instruction) {
    writeMemory(instruction, instruction.operands[0], acc_.get());
}

void Cpu::executeShw(const Instruction& instruction) {
    const std::string& operand = instruction.operands[0];

    if (operand == "ACC") {
        std::cout << acc_.get() << '\n';
        return;
    }

    if (operand == "ICR") {
        std::cout << icr_.get() << '\n';
        return;
    }

    if (operand == "MAR") {
        std::cout << mar_.get() << '\n';
        return;
    }

    if (operand == "MDR") {
        std::cout << mdr_.get() << '\n';
        return;
    }

    if (operand == "UC") {
        std::cout << ucStateToString(ucState_) << '\n';
        return;
    }

    if (!operand.empty() && !isRegisterToken(operand) && operand[0] == 'D') {
        std::cout << readMemory(instruction, operand) << '\n';
        return;
    }

    throwInstructionError(instruction, "Invalid SHW operand '" + operand + "'");
}

void Cpu::executePause() {
    ucState_ = UcState::PAUSED;
    std::string ignored;
    std::getline(std::cin, ignored);
}

void Cpu::executeEnd() {
    ucState_ = UcState::HALT;
}

const char* Cpu::ucStateToString(UcState state) {
    switch (state) {
        case UcState::FETCH:
            return "FETCH";
        case UcState::DECODE:
            return "DECODE";
        case UcState::EXECUTE:
            return "EXECUTE";
        case UcState::PAUSED:
            return "PAUSED";
        case UcState::HALT:
            return "HALT";
        default:
            return "UNKNOWN";
    }
}
