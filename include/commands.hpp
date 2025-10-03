#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include <vector>
#include <string>

enum class CommandType {
    INIT,
    CAT_FILE,
    HASH_OBJECT,
    LS_TREE,
    WRITE_TREE,
    ADD,
    STATUS,
    COMMIT,
    LOG,
    DIFF,
    BRANCH,
    CHECKOUT,
    MERGE,
    RESET,
    REVERT,
    STASH,
    UNKNOWN
};

struct CommandParams {
    CommandType command_type;
    std::vector<std::string> args;
};

// Abstract base class for all commands
class Command {
public:
    virtual void help() = 0;
    virtual void execute(std::vector<std::string>& args) = 0;
    virtual void validate(std::vector<std::string>& args) = 0;
};


#endif // COMMANDS_HPP