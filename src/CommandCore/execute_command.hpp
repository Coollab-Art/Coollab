#pragma once
#include "CommandExecutionContext_Ref.h"

namespace Lab {

auto get_context() -> CommandExecutionContext_Ref;

template<ConcreteCommand ConcreteCommandT>
void execute_command(ConcreteCommandT const& command)
{
    get_context().execute(command);
}

} // namespace Lab