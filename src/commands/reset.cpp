#include "commands/reset.hpp"

void ResetCommand::help()
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs reset --mixed <hash>");
    utils::write(utils::INFO, "usage : vcs reset --soft <hash>");
    utils::write(utils::INFO, "usage : vcs reset --hard <hash>");
    utils::write(utils::EMPTY);
}

void warning_hard_reset() {
    utils::write(utils::EMPTY);
    utils::write(utils::WARN, "Resetting will clear the current staging area.");
    utils::write(utils::WARN, "Any uncommitted changes will be lost.");
    utils::write(utils::WARN, "If you want to save your changes, press 'N' to cancel the RESET.");
    utils::write(utils::WARN, "Then, use the commit command to save your current work.");
    utils::write(utils::WARN, "Once committed, you can safely RESET.");
    
    std::string response;
    std::cout << utils::WARN << " Are you sure you want to proceed? (Y/N) : "; 
    std::cin >> response;
    
    utils::write(utils::EMPTY);

    if (response != "Y" && response != "y") {
        utils::write(utils::INFO, "Resetting cancelled. Your current changes are safe.");
        exit(0);
    }

    utils::write(utils::INFO, "Proceeding with hard RESET...");
    utils::write(utils::EMPTY);
}

void ResetCommand::validate(std::vector<std::string>& args) {
    const int args_size = args.size();

    if(args_size < 2) {
        const std::string error_msg = "Too few arguments";
        throw std::invalid_argument(error_msg);
    }

    if(args_size > 2) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }

    const std::string& flag = args[0];
    const std::string& commit_hash = args[1];

    if(flag != "--mixed" && flag != "--soft" && flag != "--hard") {
        const std::string error_msg = "Invalid flag: " + flag + ". Expected --mixed, --soft, or --hard";
        throw std::invalid_argument(error_msg);
    }

    if(CatFileCommand().get_object_type(commit_hash) != "commit") {
        const std::string error_msg = "Invalid commit hash: " + commit_hash + ". Expected a valid commit object.";
        throw std::invalid_argument(error_msg);
    }

    if(utils::is_head_detached()) {
        utils::write(utils::ERR, "You are in a detached HEAD state.");
        utils::write(utils::ERR, "To reset commits, first switch to the desired branch using:");
        utils::write(utils::ERR, "'vcs checkout <branch-name>'");
        utils::write(utils::ERR, "Then, run the reset command to undo the commits.");
        return;
    }

    const std::string cur_branch = utils::get_current_branch();

    if(!utils::is_commit_exists_on_branch(cur_branch, commit_hash)) {
        const std::string error_msg = "Commit hash " + commit_hash + " does not exist on branch '" + cur_branch + "'.";
        throw std::invalid_argument(error_msg);
    }
}

// void remove_logs(const std::string& branch, const std::string& cur_commit_hash) {
//     const std::string branch_path = config::REFS_HEAD_DIR + branch;

//     std::stringstream buffer;
//     std::ifstream file(branch_path);
//     std::string line;

//     while (std::getline(file, line)) {
//         buffer << line << "\n";
        
//         std::istringstream iss(line);
//         std::string parent_hash, commit_hash, others;
//         iss >> parent_hash >> commit_hash >> others;

//         if(cur_commit_hash == commit_hash) break;
//     }

//     std::ofstream ofs(branch_path, std::ios::binary | std::ios::trunc);
//     ofs << buffer.str();
//     ofs.close();
// }

void mixed_reset(const std::string& commit_hash) {
    // This will directly go to the commit point just like 'checkout' but do not touch the working directory, and also removes commit logs, means same stagging area where it was at "commit".
    const std::string cur_branch = utils::get_current_branch();
    const std::string cur_branch_path = config::REFS_HEAD_DIR + cur_branch;

    // remove_logs(cur_branch, commit_hash);

    const std::string tree_hash = utils::get_tree_hash_from_commit(commit_hash);

    if(tree_hash.empty() || tree_hash.size() != 40) {
        const std::string error_msg = "Corrupted commit data for branch '" + cur_branch + "'.";
        throw std::logic_error(error_msg);
    }

    std::stringstream buffer;
    utils::solve(tree_hash, buffer, ""); // get all data from tree and put into the index files

    std::string compressed = utils::compress_zlib(buffer.str());
    std::ofstream ofs(config::INDEX_FILE, std::ios::binary | std::ios::trunc);
    ofs.write(compressed.data(), compressed.size());
    ofs.close();

    std::ofstream out(cur_branch_path, std::ios::out | std::ios::trunc);

    if (!out) {
        const std::string error_msg = "Could not open file for writing: " + cur_branch_path;
        throw std::runtime_error(error_msg);
    }

    out << commit_hash;
    out.close();
}

void sort_reset(const std::string& commit_hash) {
    // This will remove the all previous commits logs, and moves head to current commit_hash. and do not touch working directory or staging area.
    
    const std::string cur_branch = utils::get_current_branch();
    const std::string cur_branch_path = config::REFS_HEAD_DIR + cur_branch;

    // remove_logs(cur_branch, commit_hash);

    std::ofstream out(cur_branch_path, std::ios::out | std::ios::trunc);

    if (!out) {
        const std::string error_msg = "Could not open file for writing: " + cur_branch_path;
        throw std::runtime_error(error_msg);
    }

    out << commit_hash;
    out.close();
}

void hard_reset(const std::string& commit_hash) {
    const std::string cur_branch = utils::get_current_branch();
    const std::string cur_branch_path = config::REFS_HEAD_DIR + cur_branch;

    // remove_logs(cur_branch, commit_hash);

    const std::string tree_hash = utils::get_tree_hash_from_commit(commit_hash);

    if(tree_hash.empty() || tree_hash.size() != 40) {
        const std::string error_msg = "Corrupted commit data.";
        throw std::logic_error(error_msg);
    }
    
    warning_hard_reset();

    std::stringstream buffer;
    utils::solve(tree_hash, buffer, ""); // adds the last commit's tree to the index file(staging area) and removes all current files.
    
    std::string compressed = utils::compress_zlib(buffer.str());
    std::ofstream ofs_index(config::INDEX_FILE, std::ios::binary | std::ios::trunc);
    ofs_index.write(compressed.data(), compressed.size());
    ofs_index.close();

    utils::clean_working_directory();

    utils::make_checkout();

    std::ofstream ofs_head(cur_branch_path, std::ios::out | std::ios::trunc);

    if (!ofs_head) {
        const std::string error_msg = "Could not open file for writing: " + cur_branch_path;
        throw std::runtime_error(error_msg);
    }

    ofs_head.write(commit_hash.data(), commit_hash.size());
    ofs_head.close();
}

void ResetCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    const std::string& flag = args[0];
    const std::string& commit_hash = args[1];

    if(flag == "--mixed") {
        mixed_reset(commit_hash);
    }
    else if(flag == "--soft") { 
        sort_reset(commit_hash);
    }
    else if(flag == "--hard") {
        hard_reset(commit_hash);
    }
    else {
        const std::string error_msg = "Invalid flag: " + flag + ". Expected --mixed, --soft, or --hard";
        throw std::logic_error(error_msg);
    }

    utils::write(utils::OK);
}