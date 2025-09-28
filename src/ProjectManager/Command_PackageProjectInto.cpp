#include "Command_PackageProjectInto.h"
#include "CommandCore/LAB_REGISTER_COMMAND.h"
#include "ProjectManager.hpp"

namespace Lab {

void Command_PackageProjectInto::execute(CommandExecutionContext_Ref const& ctx) const
{
    ctx.project_manager().package_project_into(folder_path, ctx.save_thumbnail(), options);
}

[[nodiscard]] auto Command_PackageProjectInto::to_string() const -> std::string
{
    return fmt::format("Packaging project into {}", folder_path);
}

} // namespace Lab

LAB_REGISTER_COMMAND(Lab::Command_PackageProjectInto);
