#ifndef LS_TREE_HPP
#define LS_TREE_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "commands/cat-file.hpp"
#include <sstream>

class LsTreeCommand : public Command {
private:
    void print_tree(const std::string& tree_hash);

public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // LS_TREE_HPP