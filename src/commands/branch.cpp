#include "commands/branch.hpp"

void BranchCommand::help()
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs branch");
    utils::write(utils::INFO, "usage : vcs branch <new-branch-name>");
    utils::write(utils::INFO, "usage : vcs branch <new-branch-name> <source-hash>");
    utils::write(utils::INFO, "usage : vcs branch <new-branch-name> <source-branch-name>");
    utils::write(utils::EMPTY);
}

void BranchCommand::validate(std::vector<std::string>& args) {
    const int args_size = args.size();

    if(args_size == 0) { return; }

    if(args_size > 2) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }
    
    const std::string& new_branch = args[0];

    if(!utils::is_valid_branch_name(new_branch)) {
        const std::string error_msg = "Invalid branch name: it must contain only lowercase letters ('a' to 'z') and hyphens ('-'). A hyphen cannot be at the start or end.";
        throw std::invalid_argument(error_msg);
    }

    const std::string new_branch_path = config::REFS_HEAD_DIR + new_branch;
    if(utils::is_file_exist(new_branch_path)) { 
        const std::string error_msg = "Branch '" + args[0] + "' already exists";
        throw std::invalid_argument(error_msg);
    }


    if(args_size == 2) {
        const std::string& source_branch = args[1];
        const std::string source_branch_path = config::REFS_HEAD_DIR + source_branch;

        if(utils::is_file_exist(source_branch_path)) { return; }

        const std::string& obj_hash = args[1];    
        if(!utils::is_exist_obj(obj_hash)) {
            const std::string error_msg = "Invalid arguments: branch and/or object do not exist.";
            throw std::invalid_argument(error_msg);
        }
    }
}

void BranchCommand::print_branches() {
    std::vector<std::string> branches = utils::get_all_branches(config::REFS_HEAD_DIR);

    const std::string cur_branch = utils::get_current_branch();

    utils::write(utils::OK);

    utils::write(utils::EMPTY);
    for (const std::string& branch : branches) {
        utils::write(utils::INFO, (cur_branch == branch) ? "* " : "  ", (cur_branch == branch) ? utils::get_blue_text(branch) : branch);
    }
    utils::write(utils::EMPTY);
}

void BranchCommand::create_branch_from_hash(const std::string& new_branch, const std::string& head_commit_hash) {
    // Create the new branch file in refs/heads/<new-branch>
    const std::string new_branch_path = config::REFS_HEAD_DIR + new_branch;
    std::ofstream branch_file(new_branch_path, std::ios::out | std::ios::trunc);
    if (!branch_file) {
        const std::string error_msg = "Could not create branch file: " + new_branch_path;
        throw std::runtime_error(error_msg);
    }
    branch_file << head_commit_hash;
    branch_file.close();

    // Create log file in .vcs/log/refs/heads/<new-branch>
    const std::string log_path = config::LOG_REFS_HEAD_DIR + new_branch;
    std::ofstream log_file(log_path, std::ios::out | std::ios::trunc);
    if (!log_file) {
        const std::string error_msg = "Could not create log file: " + log_path;
        throw std::runtime_error(error_msg);
    }
    log_file.close();

    utils::write(utils::OK, "Created branch '"+ new_branch +"' at commit '"+ head_commit_hash + "'");
}

void BranchCommand::create_branch_from_source(const std::string& new_branch, const std::string& parent_branch) {
    const std::string commit_hash = utils::get_commit_hash(parent_branch);

    // Check if the parent branch has any commits
    if (commit_hash.empty() || commit_hash == std::string(40, '0')) {
        const std::string error_msg = "Cannot create branch '" + new_branch + "': No commit found in '" + parent_branch + "' branch.";
        throw VCSException(error_msg);
    }

    // Create the new branch file in refs/heads/<new-branch>
    const std::string new_branch_path = config::REFS_HEAD_DIR + new_branch;
    std::ofstream branch_file(new_branch_path, std::ios::out | std::ios::trunc);
    if (!branch_file) {
        const std::string error_msg = "Could not create branch file: " + new_branch_path;
        throw std::runtime_error(error_msg);
    }
    branch_file << commit_hash;
    branch_file.close();

    // Create log file in .vcs/log/refs/heads/<new-branch>
    const std::string log_path = config::LOG_REFS_HEAD_DIR + new_branch;
    std::ofstream log_file(log_path, std::ios::out | std::ios::trunc);
    if (!log_file) {
        const std::string error_msg = "Could not create log file: " + log_path;
        throw std::runtime_error(error_msg);
    }
    log_file.close();

    utils::write(utils::OK, "Created branch '" + new_branch + "' from '" + parent_branch + "' at commit " + commit_hash);
}

void BranchCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    const int args_size = args.size();

    switch (args_size)  
    {
        case 0:
            print_branches();
            break;

        case 1:
            create_branch_from_hash(args[0], utils::get_head_commit_hash());
            break;

        case 2:
            {
                const std::string& new_branch = args[0];
                const std::string& source_branch = args[1];
                const std::string& source_hash = args[1];
                const std::string source_branch_path = config::REFS_HEAD_DIR + source_branch;
                
                if(utils::is_file_exist(source_branch_path)) {
                    create_branch_from_source(new_branch, source_branch);
                } 
                else {
                    create_branch_from_hash(new_branch, source_hash);   
                }
            }
            break;
        
        default:
            break;
    }
}