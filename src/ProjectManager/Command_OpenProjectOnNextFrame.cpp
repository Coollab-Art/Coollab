#include "Command_OpenProjectOnNextFrame.hpp"
#include "ProjectManager.hpp"

namespace Lab {

void Command_OpenProjectOnNextFrame::execute(CommandExecutionContext_Ref const& ctx) const
{
    ctx.project_manager().open_project_on_next_frame(path);
}

[[nodiscard]] auto Command_OpenProjectOnNextFrame::to_string() const -> std::string
{
    return fmt::format("Scheduling project to be open next frame \"{}\"", Cool::File::weakly_canonical(path).string());
}

} // namespace Lab

#include "CommandCore/LAB_REGISTER_COMMAND.h"
LAB_REGISTER_COMMAND(Lab::Command_OpenProjectOnNextFrame);
