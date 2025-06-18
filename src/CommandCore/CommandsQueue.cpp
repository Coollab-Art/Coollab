#include "CommandsQueue.hpp"
#include "CommandExecutionContext_Ref.h"
#include "CommandExecutor_TopLevel.h"

namespace Lab {

void CommandsQueue::push_back(Command command)
{
    std::unique_lock lock{_mutex};
    _commands.emplace_back(std::move(command));
}

void CommandsQueue::run_all_queued_commands(CommandExecutor_TopLevel const& executor, CommandExecutionContext_Ref const& ctx)
{
    std::unique_lock lock{_mutex}; // TODO(Websocket) should instead lock just while we pop a command from the queue, then unlock, then run the command (because the command might want to submit other commands to the queue), and then lock again and pop another command
    for (auto const& command : _commands)
    {
        executor.execute(command, ctx);
    }
    _commands.clear();
}

} // namespace Lab