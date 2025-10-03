#ifndef COMMAND_EXECUTOR_HPP
#define COMMAND_EXECUTOR_HPP

#include "commands.hpp"
#include "commands.hpp"
#include "commands/init.hpp"
#include "commands/cat-file.hpp"
#include "commands/hash-object.hpp"
#include "commands/ls-tree.hpp"
#include "commands/write-tree.hpp"
#include "commands/add.hpp"
#include "commands/commit.hpp"
#include "commands/status.hpp"
#include "commands/log.hpp"
#include "commands/diff.hpp"
#include "commands/branch.hpp"
#include "commands/checkout.hpp"
#include "commands/merge.hpp"
#include "commands/reset.hpp"
#include "commands/revert.hpp"
#include "commands/stash.hpp"

class CommandExecutor {
public:
    static void execute(CommandParams& command);
};

#endif // COMMAND_EXECUTOR_HPP