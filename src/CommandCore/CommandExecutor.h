#pragma once
#include "CommandExecutionContext_Ref.h"

namespace Lab {

class CommandExecutor {
public:
    explicit CommandExecutor(CommandExecutionContext_Ref ctx)
        : _ctx{std::move(ctx)}
    {
    }

    template<ConcreteCommand ConcreteCommandT>
    void execute(ConcreteCommandT&& command, bool store_in_history = true) const
    {
        _ctx.execute(std::forward<ConcreteCommandT>(command), store_in_history);
    }

    auto get_ctx() -> CommandExecutionContext_Ref& { return _ctx; }

private:
    CommandExecutionContext_Ref _ctx;
};

} // namespace Lab
