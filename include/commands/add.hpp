#ifndef ADD_HPP
#define ADD_HPP

#include "commands.hpp"
#include "commands/hash-object.hpp"
#include "utils.hpp"
#include "config.hpp"
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <regex>
#include <set>

namespace fs = std::filesystem;

class AddCommand : public Command {
public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // ADD_HPP