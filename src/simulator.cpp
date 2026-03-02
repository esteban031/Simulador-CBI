#include "simulator.h"

#include <iostream>
#include <utility>
#include <vector>

Simulator::Simulator(std::string programPath, bool verbose)
    : programPath_(std::move(programPath)), verbose_(verbose), parser_(), cpu_() {}

int Simulator::run() {
    const std::vector<Instruction> program = parser_.parseProgram(programPath_);
    if (verbose_) {
        std::cerr << "[VERBOSE] Instrucciones cargadas: " << program.size() << '\n';
    }
    cpu_.loadProgram(program);
    cpu_.setVerbose(verbose_);
    cpu_.run();
    return 0;
}
