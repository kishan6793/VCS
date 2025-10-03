#ifndef DIFF_HPP
#define DIFF_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "commands/cat-file.hpp"
#include <map>

class DiffCommand : public Command {
public:
    void help() override;
    void commit1_and_commit2_diff(const std::string commit_hash1, const std::string commit_hash2);
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // DIFF_HPP