#include "commands/stash.hpp"

struct StashEntry {
    std::string parent_hash;
    std::string commit_hash;
    std::string index_file_hash;
    std::string branch;
    std::string timestamp;
    std::string message;
};

void StashCommand::help()
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs stash -m <message>");
    utils::write(utils::INFO, "usage : vcs stash list");
    utils::write(utils::INFO, "usage : vcs stash apply <tag>");
    utils::write(utils::INFO, "usage : vcs stash pop <tag>");
    utils::write(utils::INFO, "usage : vcs stash drop <tag>");
    utils::write(utils::INFO, "usage : vcs stash show <tag>");
    utils::write(utils::EMPTY);
}

bool is_valid_tag(const std::string& tag) {
    int tag_size = tag.size();

    if(tag_size < 8) { // stash{0}
        return false;
    } 
    
    if(tag[0] != 's' || tag[1] != 't' || tag[2] != 'a' || tag[3] != 's' || tag[4] != 'h' || tag[5] != '{' || tag.back() != '}') {
        return false;
    }

    const std::string num = tag.substr(6, tag_size - 7);

    for(char ch : num) {
        if(ch >= '0' && ch <= '9');
        else { return false; }
    }

    return true;
}

void StashCommand::validate(std::vector<std::string>& args) {
    const int args_size = args.size();

    if(args_size > 2) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }

    if(args_size == 0) {
        const std::string error_msg = "No command provided for stash.";
        throw std::logic_error(error_msg);
    }
    else if(args_size == 1) {
        if(args[0] != "list") {
            const std::string error_msg = "Invalid command: " + args[0] + ".";
            throw std::invalid_argument(error_msg);
        }
    }
    else if(args_size == 2) {

        if(args[0] == "-m") { return; }

        if(args[0][0] == '-') {
            const std::string error_msg = "Unknown option: " + args[0] + ".";
            throw std::invalid_argument(error_msg);
        }

        if(args[0] != "apply" && args[0] != "pop" && args[0] != "drop" && args[0] != "show") {
            const std::string error_msg = "Invalid command: " + args[0] + ".";
            throw std::invalid_argument(error_msg);
        }

        if(!is_valid_tag(args[1])) {
            const std::string error_msg = "Invalid tag: " + args[1] + ".";
            throw std::invalid_argument(error_msg);
        }

        const std::string& tag = args[1];

        args[1] = tag.substr(6, tag.size() - 7);
    }
}

StashEntry parse_stash_log_line(const std::string& line) {
    std::istringstream ss(line);

    std::string parent_hash, commit_hash, index_file_hash, cur_branch, timestamp_str;

    // Extract fixed fields
    if (!(ss >> parent_hash >> commit_hash >> index_file_hash >> cur_branch >> timestamp_str)) {
        const std::string error_msg = "Corrupted stash log file line: " + line;
        throw VCSException(error_msg);
    }

    // The rest of the line is the commit message, possibly starting with "commit: "
    std::string message;
    std::getline(ss, message);

    // Trim leading spaces
    message.erase(0, message.find_first_not_of(" \t"));

    const std::string prefix = "commit: ";
    if (message.compare(0, prefix.size(), prefix) == 0) {
        message = message.substr(prefix.size());
    }

    StashEntry stash_entry;
    stash_entry.parent_hash = parent_hash;
    stash_entry.commit_hash = commit_hash;
    stash_entry.index_file_hash = index_file_hash;
    stash_entry.branch = cur_branch;
    stash_entry.timestamp = timestamp_str;
    stash_entry.message = message;

    return stash_entry;
}

void print_stash_list() {
    std::ifstream stash_logs(config::LOG_STASH);

    if (!stash_logs) {
        const std::string error_msg = "Could not open stash log file: " + config::LOG_STASH;
        throw std::runtime_error(error_msg);
    }

    if(utils::get_file_size(config::LOG_STASH) == 0) {
        utils::write(utils::OK, "Stash is empty.");
        return;
    }

    utils::write(utils::OK);
    utils::write(utils::EMPTY);

    std::string line;
    int line_number = 0;

    while (std::getline(stash_logs, line)) {
        if (line.empty()) continue;

        StashEntry stash_entry = parse_stash_log_line(line);

        // Print formatted stash entry
        utils::write(utils::CONTENT, "stash{" + std::to_string(line_number) + "}:", "[", utils::parse_timestamp(stash_entry.timestamp), "]", "[", "WIP on branch:", stash_entry.branch, "]", "[", "message:", stash_entry.message, "]");

        ++line_number;
    }

    utils::write(utils::EMPTY);
}

IndexEntry add_file_to_stash(const fs::path& file_path) {
    fs::path rel_path = fs::relative(file_path, fs::current_path());
    
    const std::string file_content = utils::read_file_content(file_path.string());

    std::stringstream buffer;
    buffer << file_content;

    const std::string hash = HashObjectCommand::write_obj(buffer, "blob"); 
    const std::string file_size = std::to_string(fs::file_size(file_path));
    const std::string mode = utils::get_file_mode(file_path.string());
    const std::time_t mtime = utils::get_mtime(file_path.string());

    utils::write(utils::CREATED, mode, "blob", hash, mtime, file_size, file_path.string());
    
    IndexEntry index_entry;

    index_entry.filepath = rel_path.string();
    index_entry.hash = hash;
    index_entry.size = file_size;
    index_entry.mode = mode;
    index_entry.mtime = mtime;

    return index_entry;
}

void traverse(const fs::path& path, Tree& tree, const std::set<std::string>& ignore_list) {
    if (!fs::exists(path)) {
        const std::string error_msg = "Path does not exist: " + path.string();
        throw std::logic_error(error_msg);
    }

    if (fs::is_regular_file(path)) { // Directly process a single file
        if (!utils::is_ignored(path, false, ignore_list)) {
            tree.insert(add_file_to_stash(path));
        }
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(path)) {
        if (utils::is_path_ignored(entry.path(), entry.is_directory(), ignore_list)) { continue; }

        if(!entry.is_directory()) { 
            tree.insert(add_file_to_stash(entry.path()));
        }
    }
}

std::string add_for_commit(Node* root, bool is_status_flag, std::string path) {
    if (root == NULL) return "";

    if(root->children.empty()) { // leaf-node
        const std::string blob_entry = root->index_entry.mode + " blob " + root->index_entry.hash + " " + std::to_string(root->index_entry.mtime) + " " + root->index_entry.size + " " + root->fs_name;
        return blob_entry;
    }
    
    std::time_t current_time = 0;
    int totol_size = 0;
    
    std::stringstream buffer;
    for (auto& [fs_name, child] : root->children) {
        const std::string entry = add_for_commit(child, is_status_flag, path + "/" + fs_name);
        buffer << "\n" << entry;

        current_time = std::max(current_time, child->index_entry.mtime);
        totol_size += std::stoi(child->index_entry.size);
    }

    root->index_entry.mtime = current_time;
    root->index_entry.size = std::to_string(totol_size);

    std::string hash = HashObjectCommand::write_obj(buffer, "tree");

    const std::string entry = "040000 tree " + hash + " " + std::to_string(current_time) + " " + root->index_entry.size + " "; 
    const std::string tree_entry = entry + root->fs_name;
    const std::string tree_entry_status = entry + path;
    if(is_status_flag) utils::write(utils::CREATED, tree_entry_status);
    return tree_entry;
}

void stash_it(const std::string& stash_message) {   
    const std::string cur_branch = utils::get_current_branch();
    const std::set<std::string> ignore_list = utils::load_ignore_list();

    Tree tree;

    traverse(".", tree, ignore_list);

    // push index file also
    const std::string index_compressed_data = utils::read_and_decompress(config::INDEX_FILE);
    std::stringstream index_buffer(index_compressed_data);
    const std::string index_hash = HashObjectCommand::write_obj(index_buffer, "blob"); // create blob object

    std::string tree_entry = add_for_commit(tree.root, true, ".");
    int start_hash_index = std::string("040000").size() + std::string("tree").size() + 2; // 2 space
    const std::string tree_hash = tree_entry.substr(start_hash_index, 40);

    const std::string parent_hash = utils::read_file_content(config::STASH);
    const std::string username = utils::get_username();
    const std::string timestamp = utils::get_unix_timestamp();

    std::stringstream buffer;
    buffer << "\n" << "tree " + tree_hash;
    buffer << "\n" << "parent " + parent_hash;
    buffer << "\n" << "author " << username << " " << timestamp;
    buffer << "\n" << "committer " << username << " " << timestamp;
    buffer << "\n" << stash_message;
    
    const std::string commit_hash = HashObjectCommand::write_obj(buffer, "commit"); // commiting the commit object

    std::ofstream stash_file(config::STASH, std::ios::out | std::ios::trunc);
    stash_file << commit_hash;
    stash_file.close();

    std::ofstream log_stash(config::LOG_STASH, std::ios::out | std::ios::app);
    log_stash << parent_hash << " " << commit_hash << " " << index_hash << " " << cur_branch << " " << timestamp << " commit: " << stash_message << "\n";
    log_stash.close();

    // Now Goto latest commit, means remove all updated files
    const std::string prev_tree_hash = utils::get_tree_hash_from_commit(utils::get_head_commit_hash());

    if(prev_tree_hash.empty() || prev_tree_hash.size() != 40) {
        const std::string error_msg = "Corrupted commit data for branch '" + cur_branch + "'.";
        throw std::logic_error(error_msg);
    }

    std::stringstream new_index_buffer;
    utils::solve(prev_tree_hash, new_index_buffer, ""); // get all data from tree and put into the index files

    const std::string compressed = utils::compress_zlib(new_index_buffer.str());
    std::ofstream ofs(config::INDEX_FILE, std::ios::binary | std::ios::trunc);
    ofs.write(compressed.data(), compressed.size());
    ofs.close();

    utils::clean_working_directory();

    utils::make_checkout();

    utils::write(utils::EMPTY);
    utils::write(utils::OK, commit_hash);
}

StashEntry get_stash_line(int tag) {
    std::ifstream stash_logs(config::LOG_STASH);

    if (!stash_logs) {
        const std::string error_msg = "Could not open stash log file: " + config::LOG_STASH;
        throw std::runtime_error(error_msg);
    }

    std::string line;
    int line_number = 0;

    while (std::getline(stash_logs, line)) {
        if (line.empty()) continue;

        if(tag == line_number) {
            return parse_stash_log_line(line);
        }

        ++line_number;
    }

    const std::string error_msg = "Invalid tag provided.";
    throw std::invalid_argument(error_msg);
}

void StashCommand::solve(const std::string& tree_hash, std::string path, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files) {
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

void merge_files(const std::vector<std::string>& lines1, const std::vector<std::string>& lines2, const std::string& filepath, const std::string& tag) {
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
    for (int idx = 0; idx < (int)ops.size(); ) {
        if (ops[idx].first == COMMON) {
            merged_output.push_back(ops[idx].second);
            ++idx;
        } else {
            std::vector<std::string> branch1_lines;
            std::vector<std::string> branch2_lines;
            
            // Collect all consecutive non-common operations
            while (idx < (int)ops.size() && ops[idx].first != COMMON) {
                if (ops[idx].first == BRANCH1) {
                    branch1_lines.push_back(ops[idx].second);
                } else if (ops[idx].first == BRANCH2) {
                    branch2_lines.push_back(ops[idx].second);
                }
                ++idx;
            }
            
            // Add conflict markers only if there are changes
            if (!branch1_lines.empty() && !branch2_lines.empty()) {
                merged_output.push_back("<<<<<<< CURRENT");
                for (const auto& line : branch1_lines) {
                    merged_output.push_back(line);
                }
                merged_output.push_back("=======");
                for (const auto& line : branch2_lines) {
                    merged_output.push_back(line);
                }
                merged_output.push_back(">>>>>>> stash{" + tag + "}");
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

void merge_it(const std::map<std::string, std::pair<std::string, std::string>>& commit_hash_files, const std::string& tag) {
    for(const auto& p : commit_hash_files) {
        const std::string& filepath = p.first;
        const std::string& old_file_mode = p.second.first; // mode
        const std::string& old_file_hash = p.second.second; // blob_hash

        bool file_exists = fs::exists(filepath);

        if(!file_exists) {
            // If file does not exist in working directory, restore from stash
            utils::create_file_from_blob(filepath, old_file_hash, old_file_mode);
            continue;
        }

        // If file exists, compare hashes
        const std::string new_file_hash = utils::sha1(utils::read_file_content(filepath));
        if(new_file_hash == old_file_hash) {
            // No changes, nothing to merge
            continue;
        }

        // Both files exist and differ, perform merge
        std::vector<std::string> lines_working, lines_stash;
        utils::get_lines_from_file(filepath, lines_working);
        utils::get_lines_from_blob(old_file_hash, lines_stash);

        // Only merge if both files have content
        if (!lines_stash.empty()) {
            merge_files(lines_working, lines_stash, filepath, tag);
        }
    }
}

void put_data_in_index(const std::string& index_file_hash) {    
    // Map: filepath -> tuple(hash, size, mode, mtime)
    std::map<std::string, std::tuple<std::string, std::string, std::string, std::time_t>> index_map;

    // Read and decompress the existing index file
    const std::string latest_index_decompressed_data = utils::read_and_decompress(config::INDEX_FILE);

    std::istringstream iss_latest(latest_index_decompressed_data);
    std::string line_latest;
    while (std::getline(iss_latest, line_latest)) {
        if (line_latest.empty()) continue;  // <-- Corrected: skip empty lines

        std::istringstream line_stream(line_latest);
        std::string filepath, hash, size, mode;
        std::time_t mtime;

        // Parse the line, check if all fields are present
        if (!(line_stream >> filepath >> hash >> size >> mode >> mtime)) {
            // Handle malformed line if needed
            continue;
        }

        index_map[filepath] = {hash, size, mode, mtime};
    }

    // Read and decompress the new index blob data
    std::string index_decompressed_data = utils::read_and_decompress(utils::get_object_path(index_file_hash));

    // Find null terminator separating header and content
    std::size_t null_pos = index_decompressed_data.find('\0');  // format: "blob <size>\0<content>"
    if (null_pos == std::string::npos) {
        const std::string error_msg = "Corrupted blob hash: missing null terminator";
        throw std::logic_error(error_msg);
    }

    // Extract content after null terminator
    index_decompressed_data = index_decompressed_data.substr(null_pos + 1);

    std::istringstream iss(index_decompressed_data);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.empty()) continue; 

        std::istringstream line_stream(line);
        std::string filepath, hash, size, mode;
        std::time_t mtime;

        if (!(line_stream >> filepath >> hash >> size >> mode >> mtime)) {
            // Handle malformed line if needed
            continue;
        }

        // Insert only if filepath not already in index_map
        if (index_map.find(filepath) == index_map.end()) {
            index_map[filepath] = {hash, size, mode, mtime};
        }
    }

    // Serialize the updated index map back into a string
    std::ostringstream oss;
    for (const auto& [filepath, data] : index_map) {
        const auto& [hash, size, mode, mtime] = data;
        oss << filepath << " " << hash << " " << size << " " << mode << " " << mtime << "\n";
    }

    // Compress the serialized string
    const std::string compressed_index_file = utils::compress_zlib(oss.str());

    // Write compressed data back to index file
    std::ofstream index_file(config::INDEX_FILE, std::ios::binary | std::ios::trunc);
    if (!index_file) {
        const std::string error_msg = "Failed to open index file for writing: " + config::INDEX_FILE; 
        throw std::runtime_error(error_msg);
    }
    index_file.write(compressed_index_file.data(), compressed_index_file.size());
}

void StashCommand::stash_apply(const std::string& tag) {
    const StashEntry stash_entry = get_stash_line(stoi(tag));

    const std::string& commit_hash = stash_entry.commit_hash;
    const std::string& index_file_hash = stash_entry.index_file_hash;

    put_data_in_index(index_file_hash);

    const std::string tree_hash = utils::get_tree_hash_from_commit(commit_hash);
    std::map<std::string, std::pair<std::string, std::string>> commit_hash_files; // {file_path, blob_hash}
    solve(tree_hash, "", commit_hash_files);

    merge_it(commit_hash_files, tag);

    utils::write(utils::OK, "Applied", "stash{" + tag + "}:", "[", utils::parse_timestamp(stash_entry.timestamp), "]", "[", "WIP on branch:", stash_entry.branch, "]", "[", "message:", stash_entry.message, "]");
}

StashEntry get_stash_line_and_pop(int tag) {
    std::ifstream stash_logs(config::LOG_STASH);

    if (!stash_logs) {
        const std::string error_msg = "Could not open stash log file: " + config::LOG_STASH; 
        throw std::runtime_error(error_msg);
    }

    std::vector<StashEntry> lines;
    std::string line;

    // Read all non-empty lines
    while (std::getline(stash_logs, line)) {
        if (!line.empty()) {
            lines.push_back(parse_stash_log_line(line));
        }
    }

    stash_logs.close();

    int n = lines.size();

    if(tag >= n) {
        const std::string error_msg = "Invalid tag provided.";
        throw std::invalid_argument(error_msg);
    }

    if(n == 1) {
        std::ofstream stash_head(config::STASH, std::ios::trunc);

        if (!stash_head) {
            const std::string error_msg = "Could not open stash file for writing: " + config::STASH;
            throw std::runtime_error(error_msg);
        }

        stash_head << std::string(40, '0');
        stash_head.close();

        std::ofstream stash_out(config::LOG_STASH, std::ios::trunc);

        if (!stash_out) {
            const std::string error_msg = "Could not open stash log file for writing: " + config::LOG_STASH;
            throw std::runtime_error(error_msg);
        }

        stash_out.close();

        return lines[tag];
    }

    if(tag == 0) {
        lines[tag + 1].parent_hash = std::string(40, '0');
    }
    else if(tag == n - 1) {
        std::ofstream stash_head(config::STASH, std::ios::trunc);

        if (!stash_head) {
            const std::string error_msg = "Could not open stash file for writing: " + config::STASH;
            throw std::runtime_error(error_msg);
        }

        stash_head << lines[tag - 1].commit_hash;
        stash_head.close();
    }
    else {
        lines[tag + 1].parent_hash = lines[tag - 1].commit_hash;
    }

    // Write the remaining lines back to the file
    std::ofstream stash_out(config::LOG_STASH, std::ios::trunc);

    if (!stash_out) {
        const std::string error_msg = "Could not open stash log file for writing: " + config::LOG_STASH;
        throw std::runtime_error(error_msg);
    }

    for (int i = 0; i < n; i++) {

        if(i == tag) { continue; }

        stash_out << lines[i].parent_hash << " " << lines[i].commit_hash << " " << lines[i].index_file_hash << " " << lines[i].branch << " " << lines[i].timestamp << " commit: " << lines[i].message << "\n";
    }

    stash_out.close();

    return lines[tag];
}

void StashCommand::stash_pop(const std::string& tag) {
    StashEntry stash_entry = get_stash_line_and_pop(stoi(tag));

    const std::string& commit_hash = stash_entry.commit_hash;
    const std::string& index_file_hash = stash_entry.index_file_hash;

    put_data_in_index(index_file_hash);

    const std::string tree_hash = utils::get_tree_hash_from_commit(commit_hash);
    std::map<std::string, std::pair<std::string, std::string>> commit_hash_files; // {file_path, blob_hash}
    solve(tree_hash, "", commit_hash_files);

    merge_it(commit_hash_files, tag);

    utils::write(utils::OK, "Applied and Popped", "stash{" + tag + "}:", "[", utils::parse_timestamp(stash_entry.timestamp), "]", "[", "WIP on branch:", stash_entry.branch, "]", "[", "message:", stash_entry.message, "]");
}

void stash_drop(const std::string& tag) {
    StashEntry stash_entry = get_stash_line_and_pop(stoi(tag));
    utils::write(utils::OK, "Dropped", "stash{" + tag + "}:", "[", utils::parse_timestamp(stash_entry.timestamp), "]", "[", "WIP on branch:", stash_entry.branch, "]", "[", "message:", stash_entry.message, "]");
}

void stash_show(const std::string& tag) {
    const std::string head_commit_hash = utils::get_head_commit_hash();
    const std::string stash_commit_hash = get_stash_line(stoi(tag)).commit_hash;

    DiffCommand().commit1_and_commit2_diff(stash_commit_hash, head_commit_hash);
}

void StashCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    int args_size = args.size();

    if(args_size == 1) {
        print_stash_list();
    } 
    else if(args_size == 2) {
        if(args[0] == "-m") {
            stash_it(args[1]);
        }
        else if(args[0] == "apply") {
            stash_apply(args[1]);
        }
        else if(args[0] == "pop") {
            stash_pop(args[1]);
        }
        else if(args[0] == "drop") {
            stash_drop(args[1]);
        }
        else if(args[0] == "show") {
            stash_show(args[1]);
        }
        else {
            const std::string error_msg = "Invalid command: " + args[0] + ".";
            throw std::logic_error(error_msg);
        }
    } 
    else {
        const std::string error_msg = "Invalid number of arguments for stash command.";
        throw std::logic_error(error_msg);
    }
}