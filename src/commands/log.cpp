#include "commands/log.hpp"

//     commit command implemented

// struct Commit {
//     std::string parent_hash;
//     std::string commit_hash;
//     std::string username;
//     std::time_t timestamp;
//     std::string message;
// };

std::string get_branch_and_parent(const std::string& cur_commit_hash, std::unordered_map<std::string, std::string>& commit_hash_parent) {
    const std::vector<std::string> branches = utils::get_all_branches(config::LOG_REFS_HEAD_DIR);

    std::string branch_name;

    for (const std::string& branch : branches) {
        const std::string branch_path = config::LOG_REFS_HEAD_DIR + branch;
        const std::string branch_logs_data = utils::read_file_content(branch_path);

        std::ifstream file(branch_path);
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string parent_hash, commit_hash, others;
            iss >> parent_hash >> commit_hash >> others;
            
            if (commit_hash == cur_commit_hash) {
                branch_name = branch; // Found the branch that contains the commit hash
            }

            commit_hash_parent[commit_hash] = parent_hash;      // create linked list 
        }
    }
    return branch_name; // No branch found with the given commit hash
}

Commit get_commit_data(const std::string& commit_hash) {
    if (commit_hash.empty() || commit_hash == std::string(40, '0')) {
        throw std::logic_error("Invalid or empty commit hash provided to get_commit_data.");
    }
    const std::string commit_hash_path = utils::get_object_path(commit_hash);
    
    if (!utils::is_file_exist(commit_hash_path)) {
        const std::string error_msg = "Commit object not found for hash: " + commit_hash;
        throw std::logic_error(error_msg);
    }
    
    const std::string commit_content = utils::read_and_decompress(commit_hash_path);
    
    std::istringstream iss(commit_content);
    std::string line;
    
    Commit commit;
    commit.commit_hash = commit_hash; // Set the commit hash
    while (std::getline(iss, line)) {
        if (line.find("tree ") == 0);
        else if (line.find("commit ") == 0);   
        else if (line.find("committer ") == 0);
        else if (line.find("parent ") == 0) {
            commit.parent_hash = line.substr(7); // Remove "parent "
        } 
        else if (line.find("author ") == 0) {
            size_t pos = line.find_last_of(' ');
            commit.username = line.substr(7, pos - 7); // Remove "author "
            commit.timestamp = std::stoll(line.substr(pos + 1)); // Get the timestamp
        } 
        else if (!line.empty()) {
            commit.message += line + "\n"; // Append the message
        }
    }

    // Trim trailing newline from message
    if (!commit.message.empty() && commit.message.back() == '\n') {
        commit.message.pop_back();
    }

    return commit;
}

std::pair<std::string, std::vector<Commit>> get_all_commits() {
    std::string head_commit_hash = utils::get_head_commit_hash();
    std::unordered_map<std::string, std::string> parent;
    
    std::string cur_branch = get_branch_and_parent(head_commit_hash, parent);
    const std::string head_content = utils::read_file_content(config::HEAD_FILE);
    if(head_content.find("ref: ") != std::string::npos) {
        cur_branch = utils::get_current_branch();
    }

    std::vector<Commit> all_commits;
    std::string current_commit = utils::get_commit_hash(cur_branch);

    const std::string root_hash = std::string(40, '0'); // Represents the root commit
    while (current_commit != root_hash) {
        Commit commit = get_commit_data(current_commit);
        all_commits.push_back(commit);
        current_commit = parent[current_commit]; // Move to the parent commit
    }

    return {cur_branch, all_commits};
}

void printCommits(const std::pair<std::string, std::vector<Commit>>& branch_and_commits) {
    const std::string head_commit_hash = utils::get_head_commit_hash();
    const std::string cur_branch = branch_and_commits.first;
    
    utils::write(utils::EMPTY);
    for (const Commit& c : branch_and_commits.second) {

        std::string head_indicator = (c.commit_hash == head_commit_hash) ? "(HEAD -> " + cur_branch + ")" : "";

        // Convert timestamp to readable time
        std::time_t t = c.timestamp;
        std::tm* tm_info = std::localtime(&t);
        char date_str[100];
        std::strftime(date_str, sizeof(date_str), "%a %b %d %H:%M:%S %Y", tm_info);

        utils::write(utils::INFO, "Commit  :", c.commit_hash, head_indicator);
        utils::write(utils::INFO, "Author  :", c.username);
        utils::write(utils::INFO, "Date    :", date_str);
        utils::write(utils::INFO, "Message :", c.message);
        utils::write(utils::EMPTY);
    }
    utils::write(utils::END);
}

void LogCommand::help() {}

void LogCommand::validate(std::vector<std::string>& args) {
    int args_size = args.size();

    if(args_size > 0) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);    
    }
}

void LogCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    const std::pair<std::string, std::vector<Commit>> branch_and_commits = get_all_commits();
    utils::write(utils::OK);
    printCommits(branch_and_commits);
}