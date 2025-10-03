#include "commands/revert.hpp"

void RevertCommand::help()
{
    utils::write(utils::EMPTY);
    utils::write(utils::INFO, "usage : vcs revert <hash>");
    utils::write(utils::EMPTY);
}

void RevertCommand::validate(std::vector<std::string>& args) {
    const int args_size = args.size();

    if(args_size > 1) {
        const std::string error_msg = "Too many arguments";
        throw std::invalid_argument(error_msg);
    }

    const std::string& commit_hash = args[0];

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

void RevertCommand::execute(std::vector<std::string>& args) {
    utils::create_vcs_structure();

    utils::write(utils::OK, "TODO");
}