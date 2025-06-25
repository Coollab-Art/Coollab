#include "Command_CloseApp.h"
#include "App.h"

namespace Lab {

void Command_CloseApp::execute(CommandExecutionContext_Ref const& ctx) const
{
    ctx.app().close_app(force_kill_task_in_progress);
}

} // namespace Lab

#include "CommandCore/LAB_REGISTER_COMMAND.h"

LAB_REGISTER_COMMAND(Lab::Command_CloseApp)