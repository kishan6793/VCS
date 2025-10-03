#ifndef VCS_HPP
#define VCS_HPP

#include "utils.hpp"
#include "command_parser.hpp"
#include "command_executor.hpp"

// Main application class

class VCS {
public:
    static void run(int argc, char* argv[]);
};

#endif // VCS_HPP
