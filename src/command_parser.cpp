#include "command_parser.hpp"

CommandType CommandParser::command_type(const std::string& cmd) {
    if (cmd == "init") return CommandType::INIT;
    if (cmd == "cat-file") return CommandType::CAT_FILE;
    if (cmd == "hash-object") return CommandType::HASH_OBJECT;
    if (cmd == "ls-tree") return CommandType::LS_TREE;
    if (cmd == "write-tree") return CommandType::WRITE_TREE;
    if (cmd == "add") return CommandType::ADD;
    if (cmd == "status") return CommandType::STATUS;
    if (cmd == "commit") return CommandType::COMMIT;
    if (cmd == "log") return CommandType::LOG;
    if (cmd == "diff") return CommandType::DIFF;
    if (cmd == "branch") return CommandType::BRANCH;
    if (cmd == "checkout") return CommandType::CHECKOUT;
    if (cmd == "merge") return CommandType::MERGE;
    if (cmd == "reset") return CommandType::RESET;
    if (cmd == "revert") return CommandType::REVERT;
    if (cmd == "stash") return CommandType::STASH;
    return CommandType::UNKNOWN; 
}

CommandParams CommandParser::parse(int argc, char* argv[]) {
    // Check if the command is provided
    if (argc < 2) {
        const std::string error_msg = "Usage: vcs <command> [args]. Type 'vcs help' for more information.";
        throw std::invalid_argument(error_msg);
    }

    // Parse the command
    const std::string& command_name = std::string(argv[1]);
    const CommandType cmd_type = command_type(command_name);

    if(cmd_type == CommandType::UNKNOWN) {
        const std::string error_msg = "Unknown command: " + command_name;
        throw std::invalid_argument(error_msg);
    }

    CommandParams command;
    command.command_type = cmd_type;

    for (int i = 2; i < argc; ++i) {  // Skip program name and command name
        command.args.emplace_back(std::string(argv[i])); // Collect all arguments
    }

    return command;
}