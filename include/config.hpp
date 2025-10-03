#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>

namespace config {
    const std::string VCS_DIR           = ".vcs/";
    const std::string LOG_DIR           = ".vcs/logs/";
    const std::string LOG_REFS_DIR      = ".vcs/logs/refs/";
    const std::string LOG_REFS_HEAD_DIR = ".vcs/logs/refs/heads/";
    const std::string OBJECTS_DIR       = ".vcs/objects/";
    const std::string REFS_DIR          = ".vcs/refs/";
    const std::string REFS_HEAD_DIR     = ".vcs/refs/heads/";
    const std::string HEAD_FILE         = ".vcs/HEAD";
    const std::string INDEX_FILE        = ".vcs/index";
    const std::string STASH             = ".vcs/refs/stash";
    const std::string LOG_STASH         = ".vcs/logs/refs/stash";
    const std::string VCS_IGNORE        = ".vcsignore";
}

#endif // CONFIG_HPP