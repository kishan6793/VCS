#ifndef MERGE_HPP
#define MERGE_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "exceptions/vcs-exception.hpp"
#include "commands/status.hpp"
#include <map>

class MergeCommand : public Command {
public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
    void solve(const std::string& tree_hash, std::string path, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files);
};

#endif // MERGE_HPP