#include <iostream>
#include <exception>
#include <string>

#include "simulator.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <program_path> [--verbose|-v]\n";
        return 1;
    }

    bool verbose = false;
    if (argc >= 3) {
        const std::string flag = argv[2];
        if (flag == "--verbose" || flag == "-v") {
            verbose = true;
        } else {
            std::cerr << "Unknown option: " << flag << '\n';
            std::cerr << "Usage: " << argv[0] << " <program_path> [--verbose|-v]\n";
            return 1;
        }
    }

    if (argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <program_path> [--verbose|-v]\n";
        return 1;
    }

    try {
        Simulator simulator(argv[1], verbose);
        return simulator.run();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }
}
