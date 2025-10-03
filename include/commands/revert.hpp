#ifndef REVERT_HPP
#define REVERT_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "cat-file.hpp"

class RevertCommand : public Command {
public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // REVERT_HPP