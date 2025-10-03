#include "commands/status.hpp"

void StatusCommand::help() {}

void StatusCommand::validate(std::vector<std::string>& args) {
    int args_size = args.size();

    if(args_size > 0) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }
}

void StatusCommand::process_file(const fs::path& path, const std::map<std::string, std::pair<std::string, std::string>>& index_map) {
    if(index_map.find(path.string()) == index_map.end()) {
        this->untracked_files.push_back(path.string());
    } else {
        const std::string mode = utils::get_file_mode(path.string());
        const std::string hash = utils::sha1(utils::read_file_content(path.string()));
        std::pair<std::string, std::string> p = {mode, hash};

        auto it = index_map.find(path.string());
        if(it != index_map.end() && p != it->second) {
            this->modified_files.push_back(path.string());
        }
    }
}

void StatusCommand::iterate_directory(const fs::path& path, const std::set<std::string>& ignore_list, const std::map<std::string, std::pair<std::string, std::string>>& index_map) {
    for (const auto& entry : fs::directory_iterator(path)) {
        if (utils::is_ignored(entry.path(), entry.is_directory(), ignore_list)) continue;

        if (entry.is_directory()) {
            iterate_directory(entry.path(), ignore_list, index_map);
        } else {
            fs::path rel_path = fs::relative(entry.path(), fs::current_path());
            process_file(rel_path, index_map);
        }
    }
}

void StatusCommand::check_status(){
    std::string decompressed_data = utils::read_and_decompress(config::INDEX_FILE);

    std::map<std::string, std::pair<std::string, std::string>> index_map;

    // Decompress and load existing index 
    if (!decompressed_data.empty()) {
        std::istringstream iss(decompressed_data);
        std::string line;
        while (std::getline(iss, line)) {
            std::istringstream line_stream(line);
            std::string file_path, hash, size, mode, mtime;
            line_stream >> file_path >> hash >> size >> mode >> mtime;
            index_map[file_path] = {mode, hash};    
        }
    }

    const std::set<std::string> ignore_list = utils::load_ignore_list();
    iterate_directory(".", ignore_list, index_map);

    for(const std::pair<std::string, std::pair<std::string, std::string>>& p : index_map) {
        const std::string& filepath = p.first;

        if(!fs::exists(filepath)) {
            this->deleted_files.push_back(filepath);
        }
    }
}

bool StatusCommand::is_tree_clean() {
    return (this->modified_files.empty() && this->untracked_files.empty() && this->deleted_files.empty());
}

bool StatusCommand::is_modified_files_empty() {
    return this->modified_files.empty();
}

bool StatusCommand::is_untracked_files_empty() {
    return this->untracked_files.empty();
}

bool StatusCommand::is_deleted_files_empty() {
    return this->deleted_files.empty();
}

void StatusCommand::print_modified_files() {
    if(!this->modified_files.empty()) {
        utils::write(utils::EMPTY);
        utils::write(utils::STATUS, "Changes to be added to staging area:");
        for(const auto& file : this->modified_files) {
            utils::write(utils::MODIFIED, utils::get_red_text(file));
        }
    }
}

void StatusCommand::print_untracked_files() {
    if(!this->untracked_files.empty()) {
        utils::write(utils::EMPTY);
        utils::write(utils::STATUS, "Untracked files:");
        for(const auto& file : this->untracked_files) {
            utils::write(utils::UNTRACKED, utils::get_red_text(file));
        }
    }
}

void StatusCommand::print_deleted_files() {
    if(!this->deleted_files.empty()) {
        utils::write(utils::EMPTY);
        utils::write(utils::STATUS, "Deleted files:");
        for(const auto& file : this->deleted_files) {
            utils::write(utils::DELETED, utils::get_red_text(file));
        }
    }
}

void StatusCommand::get_index_files(std::map<std::string, std::pair<std::string, std::string>>& index_files) {
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

void StatusCommand::solve(const std::string& tree_hash, std::string path, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files) {
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

void StatusCommand::get_last_commit_files(std::map<std::string, std::pair<std::string, std::string>>& last_commit_files) {
    const std::string head_commit_hash = utils::get_head_commit_hash();
    if(head_commit_hash == std::string(40, '0')) return;

    const std::string tree_hash = utils::get_tree_hash_from_commit(head_commit_hash);

    solve(tree_hash, "", last_commit_files);
}

void StatusCommand::compare_staged_and_last_commit(std::map<std::string, std::pair<std::string, std::string>>& index_files, std::map<std::string, std::pair<std::string, std::string>>& last_commit_files, std::vector<std::pair<FileState, std::string>>& changes) {
    for(const std::pair<std::string, std::pair<std::string, std::string>>& p : index_files) {
        const std::string& filepath = p.first;
        const std::string& index_new_file_mode = p.second.first; // mode
        const std::string& index_new_file_hash = p.second.second; // blob_hash

        auto it = last_commit_files.find(filepath);  
        
        if(it == last_commit_files.end()) {
            changes.push_back({FileState::NEW_FILE, filepath});
            continue;
        }

        const std::string commit_old_file_mode = it->second.first;
        const std::string commit_old_file_hash = it->second.second;

        const std::string old_str = ((commit_old_file_mode == index_new_file_mode) ? "" : commit_old_file_mode + " ") + ((index_new_file_hash == commit_old_file_hash) ? "" : commit_old_file_hash);
        const std::string new_str = ((commit_old_file_mode == index_new_file_mode) ? "" : index_new_file_mode + " ") + ((index_new_file_hash == commit_old_file_hash) ? "" : index_new_file_hash);
        const std::string str = ((index_new_file_hash == commit_old_file_hash) ? index_new_file_hash + " " : "") + ((commit_old_file_mode == index_new_file_mode) ? commit_old_file_mode : "");

        if(index_new_file_mode != commit_old_file_mode || index_new_file_hash != commit_old_file_hash) {
            changes.push_back({FileState::MODIFIED, filepath});
        }
    }    

    for(const std::pair<std::string, std::pair<std::string, std::string>>& p : last_commit_files) {
        const std::string& filepath = p.first;
        const std::string& commit_old_file_mode = p.second.first; // mode
        const std::string& commit_old_file_hash = p.second.second; // blob_hash

        auto it = index_files.find(filepath);

        if(it == index_files.end()) {
            changes.push_back({FileState::DELETED, filepath});
        }
    }
}

void StatusCommand::print_changes(const std::vector<std::pair<FileState, std::string>>& changes) {
    std::vector<std::string> new_files, modified_files, deleted_files;
    for (const auto& change : changes) {
        switch (change.first) {
            case FileState::NEW_FILE:
                new_files.push_back(change.second);
                break;
            case FileState::MODIFIED:
                modified_files.push_back(change.second);
                break;
            case FileState::DELETED:
                deleted_files.push_back(change.second);
                break;
            default:
                break;
        }
    }
    if (!new_files.empty()) {
        utils::write(utils::EMPTY);
        utils::write(utils::STATUS, "New files to be committed:");
        for (const auto& file : new_files) utils::write(utils::NEW_FILE, utils::get_light_green_text(file));
    }
    if (!modified_files.empty()) {
        utils::write(utils::EMPTY);
        utils::write(utils::STATUS, "Modified files to be committed:");
        for (const auto& file : modified_files) utils::write(utils::MODIFIED, utils::get_light_green_text(file));
    }
    if (!deleted_files.empty()) {
        utils::write(utils::EMPTY);
        utils::write(utils::STATUS, "Deleted files:");
        for (const auto& file : deleted_files) utils::write(utils::DELETED, utils::get_light_green_text(file));
    }
}

bool StatusCommand::is_working_tree_clean() {
    std::map<std::string, std::pair<std::string, std::string>> index_files; // {file_path, blob_hash}
    get_index_files(index_files);

    std::map<std::string, std::pair<std::string, std::string>> last_commit_files; // {file_path, blob_hash}
    if(utils::get_head_commit_hash() != std::string(40, '0')) get_last_commit_files(last_commit_files);

    std::vector<std::pair<FileState, std::string>> changes;

    compare_staged_and_last_commit(index_files, last_commit_files, changes);

    if(changes.empty()) {
        return true;
    } 
    else {
        return false;
    }
}

void StatusCommand::print_status() {
    const std::string branch = utils::get_current_branch();
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "On branch", utils::get_blue_text(branch));

    std::map<std::string, std::pair<std::string, std::string>> index_files; // {file_path, blob_hash}
    get_index_files(index_files);

    std::map<std::string, std::pair<std::string, std::string>> last_commit_files; // {file_path, blob_hash}
    get_last_commit_files(last_commit_files);

    std::vector<std::pair<FileState, std::string>> changes;

    compare_staged_and_last_commit(index_files, last_commit_files, changes);

    if(is_tree_clean() && changes.empty()) {
        utils::write(utils::INFO, "nothing to commit, working tree clean");
        return;
    } 
   
    print_changes(changes);
    print_modified_files();    
    print_untracked_files();
    print_deleted_files();
    
    utils::write(utils::EMPTY);
}

void StatusCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    check_status();
    utils::write(utils::OK);
    print_status();
}