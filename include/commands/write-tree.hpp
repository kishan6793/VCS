#ifndef WRITE_TREE_HPP
#define WRITE_TREE_HPP

#include "commands.hpp"
#include "commands/hash-object.hpp"
#include "models/index.hpp"
#include "models/tree.hpp"
#include "utils.hpp"
#include "config.hpp"
#include <sstream>
#include <fstream>
#include <map>
#include <set>

class WriteTreeCommand : public Command {
public:
    void help() override;
    static std::string write_tree(bool is_status_flag);
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // WRITE_TREE_HPP