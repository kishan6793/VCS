#ifndef COMMANDS_PARSER_HPP
#define COMMANDS_PARSER_HPP

#include "commands.hpp"
#include "commands.hpp"
#include "utils.hpp"
#include <vector>
#include <string>

class CommandParser {
private:
    static CommandType command_type(const std::string& cmd);

public:
    static CommandParams parse(int argc, char* argv[]);
};

#endif // COMMANDS_PARSER_HPP