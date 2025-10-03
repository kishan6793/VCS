#include "commands/merge.hpp"

void MergeCommand::help()
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs merge <branch-name>");
    utils::write(utils::EMPTY);
}

void MergeCommand::validate(std::vector<std::string>& args) {
    const int args_size = args.size();

    const std::string branch_path = config::REFS_HEAD_DIR + args[0];

    if(!fs::exists(branch_path)) {
        const std::string error_msg = "Invalid argument: " + args[0] + " branch doesn't exits";
        throw std::invalid_argument(error_msg);
    }

    if(args_size > 1) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }
}

void MergeCommand::solve(const std::string& tree_hash, std::string path, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files) {
    const std::string tree_content = utils::read_and_decompress(utils::get_object_path(tree_hash));

    // ./main.out ls-tree 6725736609ced67ff91c01194131fb5c1e96b795
    // [ OK        ]  
    // [ CONTENT   ]  tree 303 
    // [ CONTENT   ]  100644 blob ee7b91e970e57c6b795e4f25e99f2e9e16d8f3fe 1750696059 31 demo.txt 
    // [ CONTENT   ]  100755 blob c3b26da2c89459d767e20f9d0ca95d14b5e4b4d2 1750759111 2495376 main.out 
    // [ CONTENT   ]  040000 tree 8e4df253a7968ffe2641eae68d8cfce7e0048258 1750696466 65 test1 
    // [ CONTENT   ]  040000 tree 90e2141d7522fc9c83b99c3e50d3f507165ac71d 1750696477 65 test2 

    std::stringstream ss(tree_content);
    std::string line;
    std::getline(ss, line); // Read the first line (tree 286)

    while (std::getline(ss, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string mode, type, hash, mtime, size, file_name;
        iss >> mode >> type >> hash >> mtime >> size >> file_name;

        if (type == "tree") {
            solve(hash, file_name + "/", last_commit_files); // Recursive call for sub-trees
        } else if (type == "blob") {
            const std::string filepath = path + file_name;
            last_commit_files[filepath] = {mode, hash}; 
        }
    }
}

enum OpType { COMMON, BRANCH1, BRANCH2 };

void merge(const std::vector<std::string>& lines1, const std::vector<std::string>& lines2, const std::string& filepath, const std::string& branch) {
    int n = lines1.size();
    int m = lines2.size();
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    // Build LCS DP table
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (lines1[i - 1] == lines2[j - 1]) {
                dp[i][j] = 1 + dp[i - 1][j - 1];
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    // Backtrack to collect operations
    int i = n, j = m;
    std::vector<std::pair<OpType, std::string>> ops;

    while (i > 0 && j > 0) {
        if (lines1[i - 1] == lines2[j - 1]) {
            ops.push_back({COMMON, lines1[i - 1]});
            --i; --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            ops.push_back({BRANCH1, lines1[i - 1]});
            --i;
        } else {
            ops.push_back({BRANCH2, lines2[j - 1]});
            --j;
        }
    }
    while (i > 0) {
        ops.push_back({BRANCH1, lines1[i - 1]});
        --i;
    }
    while (j > 0) {
        ops.push_back({BRANCH2, lines2[j - 1]});
        --j;
    }

    std::reverse(ops.begin(), ops.end());
    std::vector<std::string> merged_output;

    // Group operations into conflict blocks
    for (int idx = 0; idx < ops.size(); ) {
        if (ops[idx].first == COMMON) {
            merged_output.push_back(ops[idx].second);
            ++idx;
        } else {
            std::vector<std::string> branch1_lines;
            std::vector<std::string> branch2_lines;
            
            // Collect all consecutive non-common operations
            while (idx < ops.size() && ops[idx].first != COMMON) {
                if (ops[idx].first == BRANCH1) {
                    branch1_lines.push_back(ops[idx].second);
                } else if (ops[idx].first == BRANCH2) {
                    branch2_lines.push_back(ops[idx].second);
                }
                ++idx;
            }
            
            // Add conflict markers only if there are changes
            if (!branch1_lines.empty() && !branch2_lines.empty()) {
                merged_output.push_back("<<<<<<< HEAD");
                for (const auto& line : branch1_lines) {
                    merged_output.push_back(line);
                }
                merged_output.push_back("=======");
                for (const auto& line : branch2_lines) {
                    merged_output.push_back(line);
                }
                merged_output.push_back(">>>>>>> " + branch);
            }
            else {
                for (const auto& line : branch1_lines) {
                    merged_output.push_back(line);
                }
                for (const auto& line : branch2_lines) {
                    merged_output.push_back(line);
                }
            }
        }
    }

    // Write to output file
    std::ofstream out(filepath);
    for (const auto& line : merged_output) {
        out << line << "\n";
    }
    out.close();
}

void merge_branch(const std::map<std::string, std::pair<std::string, std::string>>& commit_hash1_files, const std::map<std::string, std::pair<std::string, std::string>>& commit_hash2_files, const std::string& branch) {
    for(const std::pair<std::string, std::pair<std::string, std::string>>& p : commit_hash2_files) {
        const std::string& filepath = p.first;
        const std::string commit_hash2_file_mode = p.second.first; // mode
        const std::string commit_hash2_file_hash = p.second.second; // blob_hash

        if(commit_hash1_files.find(filepath) == commit_hash1_files.end()) {
            utils::create_file_from_blob(filepath, commit_hash2_file_hash, commit_hash2_file_mode);
        }
    }

    for(const std::pair<std::string, std::pair<std::string, std::string>>& p : commit_hash1_files) {
        const std::string& filepath = p.first;
        const std::string commit_hash1_file_mode = p.second.first; // mode
        const std::string commit_hash1_file_hash = p.second.second; // blob_hash

        auto it = commit_hash2_files.find(filepath);

        if(it == commit_hash2_files.end()) { continue; }

        const std::string commit_hash2_file_mode = it->second.first; // mode
        const std::string commit_hash2_file_hash = it->second.second; // blob_hash

        if(commit_hash1_file_hash == commit_hash2_file_hash) { continue; }

        std::vector<std::string> lines1, lines2;
        utils::get_lines_from_blob(commit_hash1_file_hash, lines1);
        utils::get_lines_from_blob(commit_hash2_file_hash, lines2);

        merge(lines1, lines2, filepath, branch);

        utils::write(utils::CONFLICT, filepath);
    }
}

void MergeCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    if(utils::is_head_detached()) {
        utils::write(utils::ERR, "You are in a detached HEAD state.");
        utils::write(utils::ERR, "To merge branches, first switch to the desired branch using:");
        utils::write(utils::ERR, "'vcs checkout <branch-name>'");
        utils::write(utils::ERR, "Then, run the merge command to combine changes.");
        return;
    }

    StatusCommand status_command;
    status_command.check_status();

    if(!status_command.is_tree_clean() || !status_command.is_working_tree_clean()) {
        utils::write(utils::ERR, "Your local changes to the following files would be overwritten by merge:");
        utils::write(utils::ERR, "Please commit your changes or stash them before you merge.");
        status_command.print_status();
        return;
    }

    const std::string& branch = args[0];

    const std::string head_commit_hash1 = utils::get_commit_hash(utils::get_current_branch());
    const std::string head_commit_hash2 = utils::get_commit_hash(branch);

    const std::string tree_hash1 = utils::get_tree_hash_from_commit(head_commit_hash1);
    std::map<std::string, std::pair<std::string, std::string>> commit_hash1_files; // {file_path, blob_hash}
    solve(tree_hash1, "", commit_hash1_files);

    const std::string tree_hash2 = utils::get_tree_hash_from_commit(head_commit_hash2);
    std::map<std::string, std::pair<std::string, std::string>> commit_hash2_files; // {file_path, blob_hash}
    solve(tree_hash2, "", commit_hash2_files);

    // merge 1 <- 2

    merge_branch(commit_hash1_files, commit_hash2_files, branch);
}