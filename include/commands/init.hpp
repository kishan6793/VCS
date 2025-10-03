#ifndef INIT_HPP
#define INIT_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"

class InitCommand : public Command {
public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // INIT_HPP