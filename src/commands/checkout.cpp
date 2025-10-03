#include "commands/checkout.hpp"

void CheckoutCommand::help()
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs checkout <branch-name>");
    utils::write(utils::INFO, "usage : vcs checkout <hash>");
    utils::write(utils::INFO, "usage : vcs checkout -b <branch-name>");
    utils::write(utils::INFO, "usage : vcs checkout -b <branch-name> <start-point-hash-from-any-branch>");
    utils::write(utils::EMPTY);
}


void switch_in_head_file(const std::string& branch_name) {
    std::ofstream out(config::HEAD_FILE, std::ios::out | std::ios::trunc);

    if (!out) {
        const std::string error_msg = "Could not open HEAD file for writing: " + config::HEAD_FILE;
        throw std::runtime_error(error_msg);
    }

    out << "ref: refs/heads/" << branch_name;
    out.close();
}

void CheckoutCommand::validate(std::vector<std::string>& args) {
    const int args_size = args.size();

    if(args_size < 1) {
        const std::string error_msg = "Too few arguments";
        throw std::invalid_argument(error_msg);
    }

    if(args_size > 3) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }
    
    if(args_size == 1)  {
        const std::string branch_path = config::REFS_HEAD_DIR + args[0];
        if(utils::is_valid_branch_name(args[0]) && utils::is_file_exist(branch_path)) { return; }
    
        const std::string obj_path = utils::get_object_path(args[0]);
        
        if(!utils::is_valid_hash_syntax(args[0]) || !utils::is_file_exist(obj_path)) {
            const std::string error_msg = "Invalid arguments: branch and/or object do not exist.";
            throw std::invalid_argument(error_msg);
        }
    }
    else if(args_size == 2) {
        if(args[0] != "-b") {
            const std::string error_msg = "Invalid flag. Only '-b' is supported";
            throw std::invalid_argument(error_msg);
        }
         
        const std::string branch_path = config::REFS_HEAD_DIR + args[1];
        if(!utils::is_valid_branch_name(args[1])) {
            const std::string error_msg = "Invalid arguments: branch name '" + args[1] + "' is not valid.";
            throw std::invalid_argument(error_msg);
        }

        if(utils::is_file_exist(branch_path)) {
            const std::string error_msg = "Invalid arguments: branch '" + args[1] + "' already exists.";
            throw std::invalid_argument(error_msg);
        }
    }
    else {
        if(args[0] != "-b") {
            const std::string error_msg = "Invalid flag. Only '-b' is supported";
            throw std::invalid_argument(error_msg);
        }

        const std::string branch_path = config::REFS_HEAD_DIR + args[1];
        if(!utils::is_valid_branch_name(args[1])) {
            const std::string error_msg = "Invalid arguments: branch name '" + args[1] + "' is not valid.";
            throw std::invalid_argument(error_msg);
        }

        if(utils::is_file_exist(branch_path)) {
            const std::string error_msg = "Invalid arguments: branch '" + args[1] + "' already exists.";
            throw std::invalid_argument(error_msg);
        }

        const std::string obj_path = utils::get_object_path(args[2]);    
        if(!utils::is_valid_hash_syntax(args[2]) || !utils::is_file_exist(obj_path)) {
            const std::string error_msg = "Invalid arguments: Object '" + args[2] + "' doesn't exist.";
            throw std::invalid_argument(error_msg);
        }
    }
}

void CheckoutCommand::create_branch_and_switch(const std::string& branch_name) {
    const std::string cur_branch = utils::get_current_branch();
    BranchCommand::create_branch_from_source(branch_name, cur_branch); // This will create the branch file in refs/heads/<branch_name> and puts the parent latest commit hash in it.
    switch_in_head_file(branch_name); // Update HEAD file to point to the new branch

    //  (branch) (main)
    //             O
    //             O
    //             O
    //     O-------O<---here
    //     ^
    //     |
    //    to here.
}

void CheckoutCommand::switch_to_branch(const std::string& branch_name) {
    const std::string cur_branch = utils::get_current_branch();
    if(cur_branch == branch_name) {
        utils::write(utils::OK, "Already on branch '" + branch_name + "'.");
        return;
    }
    
    utils::warning_checkout();

    const std::string branch_path = config::REFS_HEAD_DIR + branch_name;
    if(!utils::is_file_exist(branch_path)) {
        const std::string error_msg = "Branch '" + branch_name + "' does not exist";
        throw VCSException(error_msg);
    }

    const std::string commit_hash = utils::get_commit_hash(branch_name); // branch file never empty
    const std::string tree_hash = utils::get_tree_hash_from_commit(commit_hash);

    if(tree_hash.empty() || tree_hash.size() != 40) {
        const std::string error_msg = "Corrupted commit data for branch '" + branch_name + "'.";
        throw std::logic_error(error_msg);
    }

    std::stringstream buffer;
    utils::solve(tree_hash, buffer, ""); // adds the last commit's tree to the index file(staging area) and removes all current files.
    
    std::string compressed = utils::compress_zlib(buffer.str());
    std::ofstream ofs(config::INDEX_FILE, std::ios::binary | std::ios::trunc);
    ofs.write(compressed.data(), compressed.size());
    ofs.close();

    utils::clean_working_directory(); // because we are switching branches, we need to delete all files in the index file.
    
    utils::make_checkout(); // put all the files in current working directory based on the index file.
    
    switch_in_head_file(branch_name); // Update HEAD file to point to the new branch
    
    //  (branch) (main)
    //            O
    //     O------O
    //     O      O
    //     O      O<---here
    //     O
    //     O<----to here.(retreive all the staging area)

    utils::write(utils::OK, "Switched to branch '" + branch_name + "'.");
}

void CheckoutCommand::switch_to_hash(const std::string& commit_hash) {
    const std::string cur_commit_hash = utils::get_head_commit_hash(); // branch file never empty
    if(cur_commit_hash == commit_hash) {
        utils::write(utils::OK, "Already on HEAD '" + commit_hash + "'.");
        return;
    }
    
    const std::string tree_hash = utils::get_tree_hash_from_commit(commit_hash);
    if(tree_hash.empty() || tree_hash.size() != 40) {
        const std::string error_msg = "Corrupted commit data.";
        throw std::logic_error(error_msg);
    }
    
    utils::warning_checkout();

    std::stringstream buffer;
    utils::solve(tree_hash, buffer, ""); // adds the last commit's tree to the index file(staging area) and removes all current files.
    
    std::string compressed = utils::compress_zlib(buffer.str());
    std::ofstream ofs_index(config::INDEX_FILE, std::ios::binary | std::ios::trunc);
    ofs_index.write(compressed.data(), compressed.size());
    ofs_index.close();

    utils::clean_working_directory(); // because we are switching branches, we need to delete all files in the index file.

    utils::make_checkout();

    utils::write(utils::WARN, "You are in a detached HEAD state.");
    utils::write(utils::WARN, "You cannot commit in this state.");
    utils::write(utils::WARN, "To make commits, create a new branch using: ");
    utils::write(utils::WARN, "        vcs checkout -b <branch-name> <hash> [OR]");
    utils::write(utils::WARN, "        vcs branch <branch-name> <hash>");

    // Now head is Commit hash 
    std::ofstream ofs_head(config::HEAD_FILE, std::ios::binary | std::ios::trunc);
    ofs_head.write(commit_hash.data(), commit_hash.size());
    ofs_head.close();

    //  (branch) (main)
    //            O
    //     O------O
    //     O      O
    //     O      O<---here
    //     O
    //     O<----to here
    //     O
    //     O

    //  (branch) (main)
    //            O
    //            O<---to here
    //            O
    //     O------O
    //     O      O
    //     O      O<---here
    //     O
    //     O

    utils::write(utils::OK, "Switched to '" + commit_hash + "'.");
}
void CheckoutCommand::create_branch_and_switch_to_hash(const std::string& branch_name, const std::string& commit_hash) {    
    const std::string tree_hash = utils::get_tree_hash_from_commit(commit_hash);

    if(tree_hash.empty() || tree_hash.size() != 40) {
        const std::string error_msg = "Corrupted commit data.";
        throw std::logic_error(error_msg);
    }

    utils::warning_checkout();

    std::stringstream buffer;
    utils::solve(tree_hash, buffer, ""); // adds the last commit's tree to the index file(staging area) and removes all current files.
    
    std::string compressed = utils::compress_zlib(buffer.str());
    std::ofstream ofs(config::INDEX_FILE, std::ios::binary | std::ios::trunc);
    ofs.write(compressed.data(), compressed.size());
    ofs.close();

    utils::clean_working_directory(); // because we are switching branches, we need to delete all files in the index file.

    utils::make_checkout();

    // Create the new branch file in refs/heads/<new-branch>
    const std::string new_branch_path = config::REFS_HEAD_DIR + branch_name;
    std::ofstream branch_file(new_branch_path, std::ios::out | std::ios::trunc);
    if (!branch_file) {
        const std::string error_msg = "Could not create branch file: " + new_branch_path;
        throw std::runtime_error(error_msg);
    }
    branch_file << commit_hash;
    branch_file.close();

    utils::write(utils::OK, "Created branch and switched to '"+ branch_name +"' at commit " + commit_hash);

    switch_in_head_file(branch_name);
    
    //       (branch)  (main)
    //                   O
    //           O-------O
    //           ^       O
    //           |       O
    //         to here   O
    //                   O<----here 
}

void CheckoutCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    const int args_size = args.size();

    if(args_size == 1) {
        const std::string branch_path = config::REFS_HEAD_DIR + args[0];
        if(utils::is_valid_branch_name(args[0]) && utils::is_file_exist(branch_path)) { 
            switch_to_branch(args[0]); // done
        }
        else {
            switch_to_hash(args[0]);
        } 
    }
    else if(args_size == 2) {
        create_branch_and_switch(args[1]); // done
    } 
    else if(args_size == 3) {
        create_branch_and_switch_to_hash(args[1], args[2]);
    }
    else {
        const std::string error_msg = "Invalid number of arguments.";
        throw std::logic_error(error_msg);
    }
}