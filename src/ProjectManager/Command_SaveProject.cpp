#include "Command_SaveProject.h"
#include "CommandCore/LAB_REGISTER_COMMAND.h"
#include "ProjectManager.hpp"

namespace Lab {

void Command_SaveProject::execute(CommandExecutionContext_Ref const& ctx) const
{
    if (is_autosave)
        ctx.project_manager().autosave_project(must_absolutely_succeed, ctx.window_title_setter());
    else
        ctx.project_manager().save_project(must_absolutely_succeed, ctx.window_title_setter());
}

[[nodiscard]] auto Command_SaveProject::to_string() const -> std::string
{
    return is_autosave ? "Autosaving project" : "Saving project";
}

} // namespace Lab

LAB_REGISTER_COMMAND(Lab::Command_SaveProject);
