#ifndef STASH_HPP
#define STASH_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "commands/hash-object.hpp"
#include "commands/diff.hpp"
#include "models/tree.hpp"
#include "models/index.hpp"
#include "exceptions/vcs-exception.hpp"
#include <map>

class StashCommand : public Command {
private:
    void solve(const std::string& tree_hash, std::string path, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files);

public:
    void stash_pop(const std::string& tag);
    void stash_apply(const std::string& tag);
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // STASH_HPP