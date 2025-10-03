#ifndef UTILS_HPP
#define UTILS_HPP

#include "config.hpp"
#include "exceptions/vcs-exception.hpp"
#include <openssl/sha.h>
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <zlib.h>
#include <ctime>
#include <set>

namespace fs = std::filesystem;

namespace utils {

    inline constexpr const char* OK        = "[ OK        ] ";
    inline constexpr const char* INFO      = "[ INFO      ] ";
    inline constexpr const char* WARN      = "[ WARN      ] ";
    inline constexpr const char* ERR       = "[ ERROR     ] ";
    inline constexpr const char* TRUE      = "[ TRUE      ] ";
    inline constexpr const char* FALSE     = "[ FALSE     ] ";
    inline constexpr const char* CREATED   = "[ CREATED   ] ";
    inline constexpr const char* STATUS    = "[ STATUS    ] ";
    inline constexpr const char* CONTENT   = "[ CONTENT   ] ";
    inline constexpr const char* EMPTY     = "[           ] ";
    inline constexpr const char* MODIFIED  = "[ MODIFIED  ] ";
    inline constexpr const char* UNTRACKED = "[ UNTRACKED ] ";
    inline constexpr const char* REMOVED   = "[ REMOVED   ] ";
    inline constexpr const char* ADDED     = "[ ADDED     ] ";
    inline constexpr const char* BUG       = "[ BUG       ] ";
    inline constexpr const char* SYS_ERR   = "[ SYS_ERR   ] ";
    inline constexpr const char* NONE      = "[ NONE      ] ";
    inline constexpr const char* END       = "[ END       ] ";
    inline constexpr const char* DELETED   = "[ DELETED   ] ";
    inline constexpr const char* NEW_FILE  = "[ NEW FILE  ] ";
    inline constexpr const char* CONFLICT  = "[ CONFLICT  ] ";

    enum class DIR_STATUS {
        ALREADY_EXIST,
        CREATED,
        ERROR
    };

    enum class FILE_STATUS {
        ALREADY_EXIST,
        CREATED,
        ERROR
    };

    template <typename... T>
    void write(T&&... args) {
        ((std::cout << args << " "), ...);
        std::cout << '\n';
    }

    void clear_screen();

    bool is_directory_exist(const std::string& path);

    bool is_file_exist(const std::string& path);

    std::string get_current_path();

    DIR_STATUS create_directory(const std::string& path);

    FILE_STATUS create_file(const std::string& path, const std::string& content);

    void create_vcs_structure();

    std::string get_object_path(const std::string& obj_hash);

    bool is_exist_obj(const std::string& obj_hash);

    std::string read_and_decompress(const std::string& obj_path);

    bool is_valid_hash_syntax(const std::string& hash);

    std::string sha1(const std::string& input);

    std::string read_file_content(const std::string& filePath);

    std::string compress_zlib(const std::string& input);

    std::string decompress_zlib(const std::string& compressed_data);

    std::time_t get_current_timestamp();

    std::string get_file_mode(const std::string& path);

    std::time_t get_mtime(const std::string& path);

    std::string extract_ref_path(const std::string& line);

    std::string extract_ref_branch(const std::string& line);

    std::string get_username();

    std::string get_unix_timestamp();

    std::uintmax_t get_file_size(const std::string& file_path);

    bool path_exists(const std::string& relative_path);

    std::string normalizeRelativePath(const std::string& path);

    bool is_valid_branch_name(const std::string& str);

    std::string get_current_branch();

    std::string get_commit_hash(const std::string& branch_name);

    std::set<std::string> load_ignore_list();

    bool is_ignored(const fs::path& path, bool is_directory, const std::set<std::string>& ignore_list);

    std::string get_tree_hash_from_commit(const std::string& commit_hash);

    std::vector<std::string> get_all_branches(const std::string& path);

    std::string get_head_commit_hash();

    std::string get_red_text(const std::string& text);

    std::string get_green_text(const std::string& text);

    std::string get_lightblue_text(const std::string& text);

    std::string get_blue_text(const std::string& text);

    std::string get_light_green_text(const std::string& text);

    bool is_head_detached();

    void create_file_from_blob(const std::string& filepath, const std::string& hash, const std::string& mode);

    void get_lines_from_blob(const std::string& hash, std::vector<std::string>& lines);

    bool is_commit_exists_on_branch(const std::string& branch, const std::string& commit_hash);

    void warning_checkout();

    void solve(const std::string& tree_hash, std::stringstream& buffer, std::string path);

    void clean_working_directory();

    void make_checkout();

    bool is_path_ignored(const fs::path& path, bool is_directory, const std::set<std::string>& ignore_list);

    void get_lines_from_file(const std::string& path, std::vector<std::string>& lines);

    std::string parse_timestamp(const std::string& timestamp_str);
}

#endif // UTILS_HPP
