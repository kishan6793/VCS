#ifndef BRANCH_HPP
#define BRANCH_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "exceptions/vcs-exception.hpp"

class BranchCommand : public Command {
private:
    void print_branches();
    std::vector<std::string> get_all_branches(const std::string& path);
    
public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
    static void create_branch_from_source(const std::string& new_branch, const std::string& parent_branch);
    static void create_branch_from_hash(const std::string& new_branch, const std::string& head_commit_hash);
};

#endif // BRANCH_HPP