#include "cpu.h"

#include <array>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "memory_address.h"

Cpu::Cpu()
    : program_(),
      memory_(),
      acc_("ACC"),
      icr_("ICR"),
      mar_("MAR"),
      mdr_("MDR"),
      ucState_(UcState::HALT),
      verbose_(false),
      traceStream_(nullptr) {}

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

void Cpu::setTraceStream(std::ostream* stream) {
    traceStream_ = stream;
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
    const std::string message =
        "[FETCH] ICR=" + std::to_string(currentIcr) + " RAW=\"" + instruction.rawLine + "\"";
    emitTrace(message);

    if (verbose_) {
        std::cerr << message << '\n';
    }
}

void Cpu::logDecode(const Instruction& instruction) const {
    const std::string message =
        "[DECODE] opcode=" + instruction.opcode + " ACC=" + std::to_string(acc_.get()) +
        " ICR=" + std::to_string(icr_.get()) + " MAR=" + std::to_string(mar_.get()) +
        " MDR=" + std::to_string(mdr_.get());
    emitTrace(message);

    if (verbose_) {
        std::cerr << message << '\n';
    }
}

void Cpu::logExecute() const {
    const std::string message =
        "[EXECUTE] ACC=" + std::to_string(acc_.get()) + " ICR=" + std::to_string(icr_.get()) +
        " MAR=" + std::to_string(mar_.get()) + " MDR=" + std::to_string(mdr_.get()) +
        " UC=" + ucStateToString(ucState_);
    emitTrace(message);

    if (verbose_) {
        std::cerr << message << '\n';
    }
}

void Cpu::logAction(const std::string& message) const {
    emitTrace("  -> " + message);

    if (verbose_) {
        std::cerr << "  -> " << message << '\n';
    }
}

void Cpu::emitTrace(const std::string& message) const {
    if (traceStream_ == nullptr) {
        return;
    }
    (*traceStream_) << message << '\n';
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
    const MemoryAddress memoryAddress(memory_, address);
    const long long value = memoryAddress.read();
    mar_.set(static_cast<long long>(memoryAddress.index()));
    mdr_.set(value);
    return value;
}

void Cpu::writeMemory(
    const Instruction& instruction, const std::string& addressToken, long long value) {
    const std::size_t address = parseAddressToken(instruction, addressToken);
    MemoryAddress memoryAddress(memory_, address);
    memoryAddress.write(value);
    mar_.set(static_cast<long long>(memoryAddress.index()));
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
    logAction("SET: MEM[" + operands[0] + "] = " + std::to_string(value));
}

void Cpu::executeLdr(const Instruction& instruction) {
    const std::string& source = instruction.operands[0];
    const long long value = readMemory(instruction, source);
    acc_.set(value);
    logAction("LDR: ACC = MEM[" + source + "] = " + std::to_string(value));
}

void Cpu::executeAdd(const Instruction& instruction) {
    const std::array<std::string, 4>& operands = instruction.operands;
    if (isNullToken(operands[1])) {
        const long long value = readMemory(instruction, operands[0]);
        const long long previousAcc = acc_.get();
        const long long result = previousAcc + value;
        acc_.set(result);
        logAction(
            "ADD(1): ACC = " + std::to_string(previousAcc) + " + MEM[" + operands[0] +
            "](" + std::to_string(value) + ") = " + std::to_string(result));
        return;
    }

    const long long lhs = readMemory(instruction, operands[0]);
    const long long rhs = readMemory(instruction, operands[1]);
    const long long result = lhs + rhs;
    acc_.set(result);
    logAction(
        "ADD(2/3): ACC = MEM[" + operands[0] + "](" + std::to_string(lhs) + ") + MEM[" +
        operands[1] + "](" + std::to_string(rhs) + ") = " + std::to_string(result));

    if (!isNullToken(operands[2])) {
        writeMemory(instruction, operands[2], result);
        logAction("ADD(3): MEM[" + operands[2] + "] = " + std::to_string(result));
    }
}

void Cpu::executeInc(const Instruction& instruction) {
    const std::string& destination = instruction.operands[0];
    const long long value = readMemory(instruction, destination) + 1;
    writeMemory(instruction, destination, value);
    logAction("INC: MEM[" + destination + "] incremented to " + std::to_string(value));
}

void Cpu::executeDec(const Instruction& instruction) {
    const std::string& destination = instruction.operands[0];
    const long long value = readMemory(instruction, destination) - 1;
    writeMemory(instruction, destination, value);
    logAction("DEC: MEM[" + destination + "] decremented to " + std::to_string(value));
}

void Cpu::executeStr(const Instruction& instruction) {
    writeMemory(instruction, instruction.operands[0], acc_.get());
    logAction(
        "STR: MEM[" + instruction.operands[0] + "] = ACC(" + std::to_string(acc_.get()) + ")");
}

void Cpu::executeShw(const Instruction& instruction) {
    const std::string& operand = instruction.operands[0];

    if (operand == "ACC") {
        const long long value = acc_.get();
        std::cout << value << '\n';
        logAction("SHW: displaying ACC = " + std::to_string(value));
        return;
    }

    if (operand == "ICR") {
        const long long value = icr_.get();
        std::cout << value << '\n';
        logAction("SHW: displaying ICR = " + std::to_string(value));
        return;
    }

    if (operand == "MAR") {
        const long long value = mar_.get();
        std::cout << value << '\n';
        logAction("SHW: displaying MAR = " + std::to_string(value));
        return;
    }

    if (operand == "MDR") {
        const long long value = mdr_.get();
        std::cout << value << '\n';
        logAction("SHW: displaying MDR = " + std::to_string(value));
        return;
    }

    if (operand == "UC") {
        const char* value = ucStateToString(ucState_);
        std::cout << value << '\n';
        logAction(std::string("SHW: displaying UC = ") + value);
        return;
    }

    if (!operand.empty() && !isRegisterToken(operand) && operand[0] == 'D') {
        const long long value = readMemory(instruction, operand);
        std::cout << value << '\n';
        logAction("SHW: displaying MEM[" + operand + "] = " + std::to_string(value));
        return;
    }

    throwInstructionError(instruction, "Invalid SHW operand '" + operand + "'");
}

void Cpu::executePause() {
    ucState_ = UcState::PAUSED;
    const std::string pauseMessage =
        "[PAUSE] Execution paused. Enter menu commands to inspect state.";
    emitTrace(pauseMessage);
    std::cerr << pauseMessage << '\n';
    runPauseMenu();
    logAction("PAUSE: resumed");
}

void Cpu::executeEnd() {
    ucState_ = UcState::HALT;
    logAction("END: program halted");
}

void Cpu::runPauseMenu() {
    for (;;) {
        printPauseMenu();
        std::cerr << "pause> ";

        std::string line;
        if (!std::getline(std::cin, line)) {
            emitTrace("[PAUSE] Input stream closed. Resuming execution.");
            std::cerr << "[PAUSE] Input stream closed. Resuming execution.\n";
            return;
        }

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command.empty()) {
            std::cerr << "Type an option (1-5). Use 4 to continue.\n";
            continue;
        }

        if (command == "1" || command == "regs" || command == "registers") {
            showPauseRegisters();
            continue;
        }

        if (command == "2" || command == "mem") {
            std::string addressToken;
            iss >> addressToken;
            if (addressToken.empty()) {
                std::cerr << "Usage: 2 Dn   or   mem Dn\n";
                continue;
            }
            showPauseMemoryAddress(addressToken);
            continue;
        }

        if (command == "3" || command == "range") {
            std::string startToken;
            std::string endToken;
            iss >> startToken >> endToken;
            if (startToken.empty() || endToken.empty()) {
                std::cerr << "Usage: 3 Dstart Dend   or   range Dstart Dend\n";
                continue;
            }
            showPauseMemoryRange(startToken, endToken);
            continue;
        }

        if (command == "4" || command == "c" || command == "continue" || command == "resume") {
            emitTrace("[PAUSE] Continue requested.");
            return;
        }

        if (command == "5" || command == "help" || command == "?") {
            continue;
        }

        std::cerr << "Unknown option. Use 5 for help.\n";
    }
}

void Cpu::printPauseMenu() const {
    std::cerr << '\n';
    std::cerr << "=== PAUSE MENU ===\n";
    std::cerr << "1) regs                Show ACC, ICR, MAR, MDR, UC\n";
    std::cerr << "2) mem Dn              Show memory value at Dn\n";
    std::cerr << "3) range Dstart Dend   Show memory values in range\n";
    std::cerr << "4) continue            Resume execution\n";
    std::cerr << "5) help                Show this menu again\n";
}

void Cpu::showPauseRegisters() const {
    const std::string line = "REGS ACC=" + std::to_string(acc_.get()) +
                             " ICR=" + std::to_string(icr_.get()) +
                             " MAR=" + std::to_string(mar_.get()) +
                             " MDR=" + std::to_string(mdr_.get()) +
                             " UC=" + ucStateToString(ucState_);
    emitTrace("[PAUSE] " + line);
    std::cerr << line << '\n';
}

void Cpu::showPauseMemoryAddress(const std::string& token) const {
    std::size_t address = 0;
    if (!tryParseDnAddress(token, address)) {
        std::cerr << "Invalid address. Expected format Dn (e.g., D2).\n";
        return;
    }

    const MemoryAddress memoryAddress(memory_, address);
    const long long value = memoryAddress.read();
    const std::string line = "MEM[" + token + "] = " + std::to_string(value);
    emitTrace("[PAUSE] " + line);
    std::cerr << line << '\n';
}

void Cpu::showPauseMemoryRange(const std::string& startToken, const std::string& endToken) const {
    std::size_t start = 0;
    std::size_t end = 0;
    if (!tryParseDnAddress(startToken, start) || !tryParseDnAddress(endToken, end)) {
        std::cerr << "Invalid range. Expected format: Dstart Dend\n";
        return;
    }

    if (start > end) {
        std::size_t tmp = start;
        start = end;
        end = tmp;
    }

    emitTrace(
        "[PAUSE] Memory range request from D" + std::to_string(start) + " to D" +
        std::to_string(end));
    for (std::size_t address = start; address <= end; ++address) {
        const MemoryAddress memoryAddress(memory_, address);
        const long long value = memoryAddress.read();
        std::cerr << "D" << address << " = " << value << '\n';
    }
}

bool Cpu::tryParseDnAddress(const std::string& token, std::size_t& addressOut) const {
    if (!memory_.validateAddress(token)) {
        return false;
    }

    addressOut = static_cast<std::size_t>(std::stoull(token.substr(1)));
    return true;
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
