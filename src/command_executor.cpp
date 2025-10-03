#include "command_executor.hpp"

void CommandExecutor::execute(CommandParams& command) {
    std::unique_ptr<Command> cmd;

    switch (command.command_type)
    {
    case CommandType::INIT:
        cmd = std::make_unique<InitCommand>();
        break;
    case CommandType::CAT_FILE:
        cmd = std::make_unique<CatFileCommand>();
        break;
    case CommandType::HASH_OBJECT:
        cmd = std::make_unique<HashObjectCommand>();
        break;
    case CommandType::LS_TREE:
        cmd = std::make_unique<LsTreeCommand>();
        break;
    case CommandType::WRITE_TREE:
        cmd = std::make_unique<WriteTreeCommand>();
        break;
    case CommandType::ADD:
        cmd = std::make_unique<AddCommand>();
        break;
    case CommandType::COMMIT:
        cmd = std::make_unique<CommitCommand>();
        break;
    case CommandType::STATUS:
        cmd = std::make_unique<StatusCommand>();
        break;
    case CommandType::LOG:
        cmd = std::make_unique<LogCommand>();
        break;
    case CommandType::DIFF:
        cmd = std::make_unique<DiffCommand>();
        break;
    case CommandType::BRANCH:
        cmd = std::make_unique<BranchCommand>();
        break;
    case CommandType::CHECKOUT:
        cmd = std::make_unique<CheckoutCommand>();
        break;
    case CommandType::MERGE:
        cmd = std::make_unique<MergeCommand>();
        break;
    case CommandType::RESET:
        cmd = std::make_unique<ResetCommand>();
        break;
    case CommandType::REVERT:
        cmd = std::make_unique<RevertCommand>();
        break;
    case CommandType::STASH:
        cmd = std::make_unique<StashCommand>();
        break;
    default:
        const std::string error_msg = "Parser failed.";
        throw std::logic_error(error_msg);
        break;
    }

    try {
        cmd->validate(command.args);
        cmd->execute(command.args);
    }
    catch(const std::invalid_argument& e) {
        utils::write(utils::ERR, std::string(e.what()));
        cmd->help();
    }
}