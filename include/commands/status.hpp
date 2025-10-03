#ifndef STATUS_HPP
#define STATUS_HPP

#include "models/index.hpp"
#include "models/tree.hpp"
#include "commands.hpp"
#include "utils.hpp"
#include "config.hpp"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <map>

namespace fs = std::filesystem;

enum class FileState {
    NEW_FILE,
    MODIFIED,
    DELETED,
    UNKNOWN
};

class StatusCommand : public Command {
private:
    std::vector<std::string> modified_files;
    std::vector<std::string> untracked_files;
    std::vector<std::string> deleted_files;
    
    void process_file(const fs::path& path, const std::map<std::string, std::pair<std::string, std::string>>& index_map);
    void iterate_directory(const fs::path& path, const std::set<std::string>& ignore_list, const std::map<std::string, std::pair<std::string, std::string>>& index_map);
    void print_modified_files();
    void print_untracked_files();
    void print_deleted_files();
    void get_index_files(std::map<std::string, std::pair<std::string, std::string>>& index_files);
    void get_last_commit_files(std::map<std::string, std::pair<std::string, std::string>>& last_commit_files);
    void solve(const std::string& tree_hash, std::string path, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files);
    void compare_staged_and_last_commit(std::map<std::string, std::pair<std::string, std::string>>& index_files, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files, std::vector<std::pair<FileState, std::string>>& changes);
    void print_changes(const std::vector<std::pair<FileState, std::string>>& changes);

public:
    void help() override;
    void check_status();
    bool is_tree_clean();
    bool is_working_tree_clean();
    bool is_untracked_files_empty();
    bool is_modified_files_empty();
    bool is_deleted_files_empty();
    void print_status();
    void execute(std::vector<std::string>& args) override;
    void validate(std::vector<std::string>& args) override;
};

#endif // STATUS_HPP