#include "vcs.hpp"
#include "exceptions/vcs-exception.hpp"

void VCS::run(int argc, char* argv[]) {
    try {
        CommandParams command = CommandParser::parse(argc, argv);
        CommandExecutor::execute(command);
    }
    catch(const std::invalid_argument& e) {
        utils::write(utils::ERR, std::string(e.what()));
    }
    catch(const std::runtime_error& e) {
        utils::write(utils::SYS_ERR, std::string(e.what()));
    }
    catch (const VCSException& e) {
        utils::write(utils::ERR, std::string(e.what()));
    }
    catch (const std::logic_error& e) {
        utils::write(utils::BUG, std::string(e.what()));
    }
    catch(const std::exception& e) {
        utils::write(utils::ERR, e.what());
    }
}