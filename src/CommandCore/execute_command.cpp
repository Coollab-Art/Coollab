#include "execute_command.hpp"
#include "App.h"
#include "get_app.hpp"

namespace Lab {

auto get_context() -> CommandExecutionContext_Ref
{
    return internal::get_app()->command_execution_context();
}

} // namespace Lab