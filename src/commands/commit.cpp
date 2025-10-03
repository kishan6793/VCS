#include "commands/commit.hpp"

void CommitCommand::help() {
    utils::write(utils::INFO, "usage: vcs commit <message>");
    utils::write("\n");
}

void CommitCommand::validate(std::vector<std::string>& args) {
    int args_size = args.size();

    if (args_size == 0 || (args_size == 1 && args[0].empty())) {
        const std::string error_msg = "No commit message provided";
        throw std::invalid_argument(error_msg);
    }
    
    if (args_size > 1) {    
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg); 
    }

    const std::string head_file_content = utils::read_file_content(config::HEAD_FILE);
    if((head_file_content.find("ref:") == std::string::npos)) {
        utils::write(utils::ERR);
        utils::write(utils::INFO, "You are in a detached HEAD state.");
        utils::write(utils::INFO, "You cannot commit in this state.");
        utils::write(utils::INFO, "To make commits, create a new branch using: vcs checkout -b <branch-name> <hash> or vcs branch <branch-name> <hash>");
        exit(0);
    }
}

void CommitCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    StatusCommand status_command;
    status_command.check_status();
    
    if(!status_command.is_tree_clean()) {
        status_command.print_status();
        utils::write(utils::INFO, "No changes added to commit (use \"vcs add\")");
        return;
    }
    
    if(status_command.is_working_tree_clean()) {
        utils::write(utils::OK, "Everything is upto date.");
        return;
    }

    const std::string& commit_message = args[0];  // commit message is the first argument

    const std::string tree_hash = WriteTreeCommand::write_tree(false); // false - for status priting

    const std::string head_file_content = utils::read_file_content(config::HEAD_FILE);
    const std::string ref_path = config::VCS_DIR + utils::extract_ref_path(head_file_content);
    
    if (!utils::is_file_exist(ref_path)) {
        std::ofstream ref_head_file(ref_path);
        ref_head_file.close();
    }
    
    // get the latest commit hash from the branch file(./vcs/ref/heads/<branch-name>) is will become parent now
    const std::string parent_hash = utils::read_file_content(ref_path);
    const std::string username = utils::get_username();
    const std::string timestamp = utils::get_unix_timestamp();

    std::stringstream buffer;
    buffer << "\n" << "tree " + tree_hash;
    buffer << "\n" << "parent " + parent_hash;
    buffer << "\n" << "author " << username << " " << timestamp;
    buffer << "\n" << "committer " << username << " " << timestamp;
    buffer << "\n" << commit_message;
    
    // commiting the commit object
    const std::string hash = HashObjectCommand::write_obj(buffer, "commit");

    const std::string branch = utils::extract_ref_branch(head_file_content);
    const std::string ref_head_file_path = config::REFS_HEAD_DIR + branch;
    const std::string log_ref_head_file_path = config::LOG_REFS_HEAD_DIR + branch;

    // Putting latest commit hash into the ./vcs/ref/<brach-name> file by trucating the <branch-name> file
    std::ofstream head_branch_file(ref_head_file_path, std::ios::out);
    if (!head_branch_file) {
        std::string error_msg = "Failed to open file: " + ref_head_file_path;
        throw std::runtime_error(error_msg);
    }

    head_branch_file << hash;
    head_branch_file.close();

    // Appending logs to the ./vcs/logs/refs/heads/<branch-name> file
    std::ofstream log_head_branch_file(log_ref_head_file_path, std::ios::out | std::ios::app);
    log_head_branch_file << parent_hash << " " << hash << " " << username << " " << timestamp << " commit: " << commit_message << "\n";
    log_head_branch_file.close();

    utils::write(utils::OK, hash);
}