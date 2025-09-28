#include "Command_Meshing.h"
#include "App.h"

namespace Lab {

auto Command_Meshing::to_string() const -> std::string
{
    return fmt::format("Convert SDF to Mesh, from node {}", reg::to_string(node_id));
}

void Command_Meshing::execute(CommandExecutionContext_Ref const& ctx) const
{
    ctx.app().open_meshing_window_for_node(node_id);
}

} // namespace Lab

#include "CommandCore/LAB_REGISTER_COMMAND.h"
LAB_REGISTER_COMMAND(Lab::Command_Meshing)