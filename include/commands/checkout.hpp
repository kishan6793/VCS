#ifndef CHECKOUT_HPP
#define CHECKOUT_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "commands/branch.hpp"
#include "exceptions/vcs-exception.hpp"
#include "models/tree.hpp"
#include <unordered_map>
#include <set>
#include <filesystem>

namespace fs = std::filesystem;

class CheckoutCommand : public Command {
private:
    void create_branch_and_switch(const std::string& branch_name);
    void switch_to_branch(const std::string& branch_name);
    void switch_to_hash(const std::string& hash);
    void create_branch_and_switch_to_hash(const std::string& branch_name, const std::string& hash);

public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // CHECKOUT_HPP