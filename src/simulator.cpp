#include "simulator.h"

#include <cerrno>
#include <cstring>
#include <direct.h>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
std::string extractStem(const std::string& path) {
    std::size_t lastSlash = path.find_last_of("/\\");
    const std::string filename =
        (lastSlash == std::string::npos) ? path : path.substr(lastSlash + 1);

    std::size_t lastDot = filename.find_last_of('.');
    if (lastDot == std::string::npos || lastDot == 0) {
        return filename;
    }
    return filename.substr(0, lastDot);
}
}  // namespace

Simulator::Simulator(std::string programPath, bool verbose)
    : programPath_(std::move(programPath)), verbose_(verbose), parser_(), cpu_() {}

int Simulator::run() {
    const std::vector<Instruction> program = parser_.parseProgram(programPath_);

    if (_mkdir("outputs") != 0 && errno != EEXIST) {
        throw std::runtime_error(
            std::string("Could not create outputs directory: ") + std::strerror(errno));
    }

    const std::string tracePath = "outputs/" + extractStem(programPath_) + "_trace.txt";
    std::ofstream traceFile(tracePath.c_str(), std::ios::trunc);
    if (!traceFile.is_open()) {
        throw std::runtime_error("Could not open trace output file: " + tracePath);
    }

    if (verbose_) {
        std::cerr << "[VERBOSE] Instrucciones cargadas: " << program.size() << '\n';
    }

    traceFile << "Program: " << programPath_ << '\n';
    traceFile << "Instructions loaded: " << program.size() << '\n';

    cpu_.loadProgram(program);
    cpu_.setVerbose(verbose_);
    cpu_.setTraceStream(&traceFile);
    cpu_.run();
    traceFile << "Execution finished.\n";
    return 0;
}
