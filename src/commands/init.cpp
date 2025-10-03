#include "commands/init.hpp"

void InitCommand::help()
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs init");
    utils::write(utils::EMPTY);
}

void InitCommand::validate(std::vector<std::string>& args) {
    const int args_size = args.size();

    if(args_size > 0) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }
}

void InitCommand::execute(std::vector<std::string>& args) {
    const bool is_reinit = utils::is_directory_exist(config::VCS_DIR);
    const std::string init_msg_1 = is_reinit ? "Reinitializing" : "Initializing";
    const std::string init_msg_2 = is_reinit ? "Reinitialized" : "Initialized";

    utils::write(utils::INFO, init_msg_1, "VCS repository...");
    
    utils::create_vcs_structure();

    const std::string cur_path = utils::get_current_path() + "/.vcs/";
    utils::write(utils::OK, init_msg_2, "empty VCS repository in", cur_path);
}