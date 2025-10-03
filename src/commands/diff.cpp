#include "commands/diff.hpp"

// echo "hello" > newfile.txt
// git diff         # won't show newfile.txt
// git status       # shows newfile.txt as untracked
// git add newfile.txt
// git diff         # still doesn’t show it (now it's staged)
// git diff --cached # shows newfile.txt as new addition

namespace fs = std::filesystem;

void DiffCommand::help() 
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage: vcs diff");                       //  (Staging Area to Working Directory) -> this will not show the new files only modified and deleted files only 
    utils::write(utils::INFO, "usage: vcs diff --staged");              //  (Staging Area vs Last Commit)
    utils::write(utils::INFO, "usage: vcs diff --cached");              //  (Staging Area vs Last Commit)
    // utils::write(utils::INFO, "usage: vcs diff <commit1>");             //  commit to current working directory
    utils::write(utils::INFO, "usage: vcs diff <branch1> <branch2>");   //  (Branch1 vs Branch2)
    utils::write(utils::INFO, "usage: vcs diff <commit1> <commit2>");   //  (Commit1 vs Commit2)
    utils::write(utils::EMPTY);
}

void DiffCommand::validate(std::vector<std::string>& args) {
    int args_size = args.size();

    if(args_size == 0);
    else if(args_size == 1) {
        if(args[0] == "--staged" || args[0] == "--cached") { return; }

        const std::string& commit_hash = args[0];

        if(CatFileCommand().get_object_type(commit_hash) != "commit") {
            const std::string error_msg = "Invalid commit hash: " + commit_hash;
            throw std::invalid_argument(error_msg);
        }
    }
    else if(args_size == 2) {
        const std::string branch1_path = config::REFS_HEAD_DIR + args[0];
        const std::string branch2_path = config::REFS_HEAD_DIR + args[1];

        if(fs::exists(branch1_path) && fs::exists(branch2_path)) { return; }

        const std::string& commit_hash1 = args[0];
        const std::string& commit_hash2 = args[1];

        if(CatFileCommand().get_object_type(commit_hash1) != "commit") {
            const std::string error_msg = "Invalid commit hash: " + commit_hash2;
            throw std::invalid_argument(error_msg);
        }

        if(CatFileCommand().get_object_type(commit_hash2) != "commit") { 
            const std::string error_msg = "Invalid commit hash: " + commit_hash2;
            throw std::invalid_argument(error_msg);
        }
    }
    else {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }
}

void get_index_files(std::map<std::string, std::pair<std::string, std::string>>& index_files) {
    const std::string index_content = utils::read_and_decompress(config::INDEX_FILE);
    std::istringstream index_stream(index_content);
    std::string line;
    while(std::getline(index_stream, line)) {
        if(line.empty()) continue; // Skip empty lines
        std::istringstream line_stream(line);
        // <file-path> <sha1-hash> <size> <mode> <mtime>
        std::string file_path, blob_hash, size, mode, mtime;
        if(line_stream >> file_path >> blob_hash >> size >> mode >> mtime) {
            index_files[file_path] = {mode, blob_hash};
        }
    }
}

void solve(const std::string& tree_hash, std::string path, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files) {
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

void get_last_commit_files(std::map<std::string, std::pair<std::string, std::string>>& last_commit_files) {
    const std::string head_commit_hash = utils::get_head_commit_hash();
    const std::string tree_hash = utils::get_tree_hash_from_commit(head_commit_hash);

    solve(tree_hash, "", last_commit_files);
}

void show_line_diff(const std::vector<std::string>& old_lines, const std::vector<std::string>& new_lines) {
    int n = old_lines.size();
    int m = new_lines.size();

    // Use vector of vectors for DP table
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    // Build LCS DP table
    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (old_lines[i - 1] == new_lines[j - 1]) {
                dp[i][j] = 1 + dp[i - 1][j - 1];
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    int delete_count = 0;
    int add_count = 0;
    int mx_size_int = std::max(std::to_string(n).size(), std::to_string(m).size());

    // Backtrack to get the diff output
    int i = n, j = m;
    std::vector<std::string> output;

    while (i > 0 && j > 0) {
        std::string old_str = std::to_string(i);
        std::string new_str = std::to_string(j);

        std::string old_empty = std::string(mx_size_int - (int)old_str.size(), ' ');
        std::string new_empty = std::string(mx_size_int - (int)new_str.size(), ' ');

        old_str = "-" + old_str + old_empty;
        new_str = "+" + new_str + new_empty;

        if (old_lines[i - 1] == new_lines[j - 1]) {
            // Lines are the same, no diff needed
            output.push_back(old_str + " " + new_str + " |   " + old_lines[i - 1]);
            --i; --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            // Line removed from old_lines
            const std::string old_line = old_str + "  " + std::string(mx_size_int, '#') + " | - " + old_lines[i - 1];
            output.push_back(utils::get_red_text(old_line));
            --i;
            ++delete_count;
        } else {
            // Line added in new_lines 
            const std::string new_line = " " + std::string(mx_size_int, '#') + " " + new_str + " | + " + new_lines[j - 1];
            output.push_back(utils::get_light_green_text(new_line));
            --j;
            ++add_count;
        }
    }

    // Remaining lines in old_lines are deletions
    while (i > 0) {
        std::string old_str = std::to_string(i);
        std::string old_empty = std::string(mx_size_int - (int)old_str.size(), ' ');
        old_str = "-" + old_str + old_empty;

        const std::string old_line = old_str + "  " + std::string(mx_size_int, '#') + " | - " + old_lines[i - 1];
        output.push_back(utils::get_red_text(old_line));
        --i;
        ++delete_count;
    }

    // Remaining lines in new_lines are additions
    while (j > 0) {
        std::string new_str = std::to_string(j);
        std::string new_empty = std::string(mx_size_int - (int)new_str.size(), ' ');
        new_str = "+" + new_str + new_empty;

        const std::string new_line = " " + std::string(mx_size_int, '#') + " " + new_str + " | + " + new_lines[j - 1];
        output.push_back(utils::get_light_green_text(new_line));
        --j;
        ++add_count;
    }

    // Reverse to get correct order
    std::reverse(output.begin(), output.end());

    utils::write(utils::INFO, "lines:", "-" + std::to_string(delete_count), "+" + std::to_string(add_count));
    
    // Print the diff lines
    for (const std::string& line : output) {
        utils::write(utils::CONTENT, line);
    }
}

void print_diff(std::map<std::string, std::pair<std::string, std::string>>& index_files) {
    utils::write(utils::OK);
    utils::write(utils::EMPTY);
    // for(const auto& [filepath, old_hash] : index_files) {
    for(const std::pair<std::string, std::pair<std::string, std::string>>& p : index_files) {
        const std::string& filepath = p.first;
        const std::string& mode = p.second.first; // mode
        const std::string& old_hash = p.second.second; // blob_hash

        if (!fs::exists(filepath)) {
            utils::write(utils::INFO, "diff:", "a/" + filepath, "b/" + filepath, old_hash, mode);
            utils::write(utils::INFO, "deleted:", filepath);
            utils::write(utils::EMPTY);
            continue;
        }

        const std::string new_hash = utils::sha1(utils::read_file_content(filepath)); 

        const std::string new_file_mode = utils::get_file_mode(filepath);

        if (mode != new_file_mode) {
            utils::write(utils::INFO, "diff:", "a/" + filepath, "b/" + filepath, old_hash);
            utils::write(utils::INFO, "old mode:", mode);
            utils::write(utils::INFO, "new mode:", new_file_mode);
            if(old_hash == new_hash) { utils::write(utils::EMPTY); }
        }

        if(old_hash == new_hash) { continue; }

        std::vector<std::string> new_lines, old_lines;
        utils::get_lines_from_file(filepath, new_lines);
        utils::get_lines_from_blob(old_hash, old_lines);

        if(mode == new_file_mode) utils::write(utils::INFO, "diff:", "a/" + filepath, "b/" + filepath, mode);
        utils::write(utils::INFO, "---", "a/" + filepath);
        utils::write(utils::INFO, "+++", "b/" + filepath);
        show_line_diff(old_lines, new_lines);
        utils::write(utils::EMPTY);
    }
}

void compare_diffs(std::map<std::string, std::pair<std::string, std::string>>& index_files, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files) {
    utils::write(utils::OK);
    utils::write(utils::EMPTY);

    for(const std::pair<std::string, std::pair<std::string, std::string>>& p : index_files) {
        const std::string& filepath = p.first;
        const std::string& index_new_file_mode = p.second.first; // mode
        const std::string& index_new_file_hash = p.second.second; // blob_hash

        auto it = last_commit_files.find(filepath);
        
        if(it == last_commit_files.end()) {
            utils::write(utils::INFO, "diff:", "a/" + filepath, "b/" + filepath, index_new_file_hash, index_new_file_mode);
            utils::write(utils::INFO, "new file:", filepath);
            utils::write(utils::EMPTY);
            continue;
        }

        const std::string commit_old_file_mode = it->second.first;
        const std::string commit_old_file_hash = it->second.second;

        const std::string old_str = ((commit_old_file_mode == index_new_file_mode) ? "" : commit_old_file_mode + " ") + ((index_new_file_hash == commit_old_file_hash) ? "" : commit_old_file_hash);
        const std::string new_str = ((commit_old_file_mode == index_new_file_mode) ? "" : index_new_file_mode + " ") + ((index_new_file_hash == commit_old_file_hash) ? "" : index_new_file_hash);
        const std::string str = ((index_new_file_hash == commit_old_file_hash) ? index_new_file_hash + " " : "") + ((commit_old_file_mode == index_new_file_mode) ? commit_old_file_mode : "");

        if(index_new_file_mode != commit_old_file_mode) {
            utils::write(utils::INFO, "diff:", "a/" + filepath, "b/" + filepath, str);
            utils::write(utils::INFO, "old:", old_str);
            utils::write(utils::INFO, "new:", new_str);
            if(index_new_file_hash == commit_old_file_hash) { utils::write(utils::EMPTY); }
        }

        if(index_new_file_hash == commit_old_file_hash) { continue; }

        std::vector<std::string> new_lines, old_lines;
        utils::get_lines_from_blob(index_new_file_hash, new_lines);
        utils::get_lines_from_blob(commit_old_file_hash, old_lines);

        if(index_new_file_mode == commit_old_file_mode) utils::write(utils::INFO, "diff:", "a/" + filepath, "b/" + filepath, str);
        if(index_new_file_mode == commit_old_file_mode) utils::write(utils::INFO, "old:", old_str);
        if(index_new_file_mode == commit_old_file_mode) utils::write(utils::INFO, "new:", new_str);
        utils::write(utils::INFO, "---", "a/" + filepath);
        utils::write(utils::INFO, "+++", "b/" + filepath);
        show_line_diff(old_lines, new_lines);
        utils::write(utils::EMPTY);
    }    

    for(const std::pair<std::string, std::pair<std::string, std::string>>& p : last_commit_files) {
        const std::string& filepath = p.first;
        const std::string commit_old_file_mode = p.second.first; // mode
        const std::string commit_old_file_hash = p.second.second; // blob_hash

        auto it = index_files.find(filepath);

        if(it == index_files.end()) {
            utils::write(utils::INFO, "diff:", "a/" + filepath, "b/" + filepath, commit_old_file_hash, commit_old_file_mode);
            utils::write(utils::INFO, "deleted file:", filepath);
            utils::write(utils::EMPTY);
        }
    }
}

void DiffCommand::commit1_and_commit2_diff(const std::string commit_hash1, const std::string commit_hash2) {
    const std::string tree_hash1 = utils::get_tree_hash_from_commit(commit_hash1);
    std::map<std::string, std::pair<std::string, std::string>> commit_hash1_files; // {file_path, blob_hash}
    solve(tree_hash1, "", commit_hash1_files);

    const std::string tree_hash2 = utils::get_tree_hash_from_commit(commit_hash2);
    std::map<std::string, std::pair<std::string, std::string>> commit_hash2_files; // {file_path, blob_hash}
    solve(tree_hash2, "", commit_hash2_files);

    compare_diffs(commit_hash1_files, commit_hash2_files);
}

void DiffCommand::execute(std::vector<std::string>& args) {
    int args_size = args.size();

    if(args_size == 0) {
        std::map<std::string, std::pair<std::string, std::string>> index_files; // {file_path, blob_hash}
        get_index_files(index_files);
        print_diff(index_files);
    }
    else if(args_size == 1) {
        std::map<std::string, std::pair<std::string, std::string>> index_files; // {file_path, blob_hash}
        get_index_files(index_files);

        std::map<std::string, std::pair<std::string, std::string>> last_commit_files; // {file_path, blob_hash}
        get_last_commit_files(last_commit_files);

        compare_diffs(index_files, last_commit_files);
    }
    else if(args_size == 2) {
        const std::string branch1_path = config::REFS_HEAD_DIR + args[0];
        const std::string branch2_path = config::REFS_HEAD_DIR + args[1];

        std::string commit_hash1;
        std::string commit_hash2;

        if(fs::exists(branch1_path) && fs::exists(branch2_path)) { 
            commit_hash1 = utils::read_file_content(branch1_path);
            commit_hash2 = utils::read_file_content(branch2_path);
        } 
        else {   
            commit_hash1 = args[0];
            commit_hash2 = args[1];
        }
            
        commit1_and_commit2_diff(commit_hash1, commit_hash2);
    }
}