#ifndef COMMIT_HPP
#define COMMIT_HPP

#include "commands.hpp"
#include "commands/write-tree.hpp"
#include "commands/hash-object.hpp"
#include "commands/status.hpp"
#include "utils.hpp"
#include "config.hpp"
#include <sstream>
#include <fstream>

class CommitCommand : public Command {
public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // COMMIT_HPP