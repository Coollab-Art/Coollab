#pragma once
#include <mutex>
#include "Command.h"

namespace Lab {

class CommandExecutor_TopLevel;
class CommandExecutionContext_Ref;

class CommandsQueue {
public:
    void push_back(Command);

private:
    friend class App;

    void run_all_queued_commands(CommandExecutor_TopLevel const&, CommandExecutionContext_Ref const&);

private:
    std::vector<Command> _commands;
    std::mutex           _mutex;
};

inline auto commands_queue() -> CommandsQueue&
{
    static auto instance = CommandsQueue{};
    return instance;
}

} // namespace Lab