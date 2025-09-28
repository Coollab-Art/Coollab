#pragma once
#include "CommandExecutionContext_Ref.h"
#include "ConcreteReversibleCommand.h"

namespace Lab {

class ReversibleCommandExecutor_WithoutHistory_Ref {
public:
    explicit ReversibleCommandExecutor_WithoutHistory_Ref(CommandExecutionContext_Ref context)
        : _context{context}
    {
    }

    /// To simplify the life of users so they don't need to call make_reversible_command()
    template<ConcreteReversibleCommand ConcreteReversibleCommandT>
    void execute(ConcreteReversibleCommandT&& command) const
    {
        execute(make_reversible_command(std::forward<ConcreteReversibleCommandT>(command)));
    }

    /// To simplify the life of users so they don't need to call make_reversible_command()
    template<ConcreteReversibleCommand ConcreteReversibleCommandT>
    void revert(ConcreteReversibleCommandT&& command) const
    {
        revert(make_reversible_command(std::forward<ConcreteReversibleCommandT>(command)));
    }

    void execute(ReversibleCommand const& command)
    {
        command->execute(_context);
        if (DebugOptions::log_when_executing_a_command())
            Cool::Log::info("Command", command->to_string());
    }

    void revert(ReversibleCommand const& command)
    {
        command->revert(_context);
        if (DebugOptions::log_when_executing_a_command())
            Cool::Log::info("Command", command->to_string());
    }

private:
    CommandExecutionContext_Ref _context;
};

} // namespace Lab