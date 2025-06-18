#include "Command_Log.h"
#include "Cool/Log/Log.hpp"

namespace Lab {

void Command_Log::execute(CommandExecutionContext_Ref const&) const
{
    Cool::Log::info(title, content);
}

} // namespace Lab

#include "CommandCore/LAB_REGISTER_COMMAND.h"

LAB_REGISTER_COMMAND(Lab::Command_Log)