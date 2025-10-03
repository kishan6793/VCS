#ifndef LOG_HPP
#define LOG_HPP

#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include "exceptions/vcs-exception.hpp"
#include <unordered_map>

struct Commit {
    std::string parent_hash;
    std::string commit_hash;
    std::string username;
    std::time_t timestamp;
    std::string message;
};

class LogCommand : public Command {
public:
    void help() override;
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // LOG_HPP