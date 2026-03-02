#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "instruction.h"
#include "memory.h"
#include "register.h"

class Cpu {
public:
    enum class UcState { FETCH, DECODE, EXECUTE, PAUSED, HALT };

    Cpu();

    void loadProgram(const std::vector<Instruction>& program);
    void setVerbose(bool verbose);
    void setTraceStream(std::ostream* stream);
    void run();

private:
    [[noreturn]] void throwInstructionError(
        const Instruction& instruction, const std::string& message) const;
    bool isNullToken(const std::string& token) const;
    bool isRegisterToken(const std::string& token) const;
    const Instruction* fetchNextInstruction();
    void logFetch(const Instruction& instruction, long long currentIcr) const;
    void logDecode(const Instruction& instruction) const;
    void logExecute() const;
    void logAction(const std::string& message) const;
    void emitTrace(const std::string& message) const;
    std::size_t parseAddressToken(const Instruction& instruction, const std::string& token) const;
    long long parseImmediateToken(const Instruction& instruction, const std::string& token);
    long long readMemory(const Instruction& instruction, const std::string& addressToken);
    void writeMemory(
        const Instruction& instruction, const std::string& addressToken, long long value);
    void decodeInstruction(const Instruction& instruction) const;
    void decodeSet(const Instruction& instruction) const;
    void decodeSingleAddress(const Instruction& instruction, const std::string& opcode) const;
    void decodeAdd(const Instruction& instruction) const;
    void decodeShw(const Instruction& instruction) const;
    void executeInstruction(const Instruction& instruction);
    void executeSet(const Instruction& instruction);
    void executeLdr(const Instruction& instruction);
    void executeAdd(const Instruction& instruction);
    void executeInc(const Instruction& instruction);
    void executeDec(const Instruction& instruction);
    void executeStr(const Instruction& instruction);
    void executeShw(const Instruction& instruction);
    void executePause();
    void executeEnd();
    static const char* ucStateToString(UcState state);

    std::vector<Instruction> program_;
    Memory memory_;
    Register acc_;
    Register icr_;
    Register mar_;
    Register mdr_;
    UcState ucState_;
    bool verbose_;
    std::ostream* traceStream_;
};
