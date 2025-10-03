#include "commands/write-tree.hpp"

void WriteTreeCommand::help() 
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs write-tree [-s]");
    utils::write(utils::INFO, "flag  : -s (status flag)");
    utils::write(utils::EMPTY);
}

void WriteTreeCommand::validate(std::vector<std::string>& args) {
    const int args_size = args.size();

    if(args_size == 0) return;

    if(args_size > 1) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }

    if(args[0] != "-s") {
        const std::string error_msg = "Invalid flag. Only '-s' is supported";
        throw std::invalid_argument(error_msg);
    }
}

std::string solve(Node* root, bool is_status_flag, std::string path) {
    if (root == NULL) return "";

    if(root->children.empty()) { // leaf-node
        const std::string blob_entry = root->index_entry.mode + " blob " + root->index_entry.hash + " " + std::to_string(root->index_entry.mtime) + " " + root->index_entry.size + " " + root->fs_name;
        return blob_entry;
    }
    
    std::time_t current_time = 0;
    int totol_size = 0;
    
    std::stringstream buffer;
    for (auto& [fs_name, child] : root->children) {
        const std::string entry = solve(child, is_status_flag, path + "/" + fs_name);
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

std::string WriteTreeCommand::write_tree(const bool is_status_flag) {
    Tree tree;

    // Decompress and load existing index 
    std::string decompressed_data = utils::read_and_decompress(config::INDEX_FILE); 
    
    if (decompressed_data.empty()) { return ""; }
    
    std::istringstream iss(decompressed_data);
    std::string line;
    while (std::getline(iss, line)) {
        std::istringstream line_stream(line);
        IndexEntry index_entry;
        line_stream >> index_entry.filepath >> index_entry.hash >> index_entry.size >> index_entry.mode >> index_entry.mtime;
        tree.insert(index_entry);
    }
    
    std::string tree_entry = solve(tree.root, is_status_flag, ".");
    
    int start_hash_index = std::string("040000").size() + std::string("tree").size() + 2; // 2 space
    const std::string tree_hash = tree_entry.substr(start_hash_index, 40);

    return tree_hash;
}

void WriteTreeCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    const bool is_status_flag = (args.size() == 1 && args[0] == "-s");

    const std::string tree_hash = write_tree(is_status_flag);
    const std::string msg = "Alredy upto date. No changes to commit";

    utils::write(utils::OK, tree_hash.empty() ? msg : tree_hash);

    // std::ofstream clear_index(config::INDEX_FILE, std::ios::out | std::ios::trunc);
    // clear_index.close();
}