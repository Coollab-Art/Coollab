#include "CommandExecutionContext_Ref.h"
#include "App.h"
#include "CommandExecutor.h"
#include "Nodes/make_sure_node_uses_the_most_up_to_date_version_of_its_definition.h"
#include "ProjectManager/Interfaces.hpp"
#include "ProjectManager/ProjectManager.hpp"

namespace Lab {

auto CommandExecutionContext_Ref::project() const -> Project&
{
    return _data.project_manager.get().project();
}

auto CommandExecutionContext_Ref::window_title_setter() const -> SetWindowTitle
{
    return _data.app.get().make_window_title_setter();
}

auto CommandExecutionContext_Ref::on_project_loaded() const -> OnProjectLoaded
{
    return _data.app.get().make_on_project_loaded();
}

auto CommandExecutionContext_Ref::save_thumbnail() const -> SaveThumbnail
{
    return _data.app.get().make_save_thumbnail();
}

auto CommandExecutionContext_Ref::history() const -> History const&
{
    return project().history;
}
auto CommandExecutionContext_Ref::camera_manager() const -> Camera3DManager&
{
    return project().camera_3D_manager;
}
auto CommandExecutionContext_Ref::command_executor() const -> CommandExecutor
{
    return CommandExecutor{*this};
}
auto CommandExecutionContext_Ref::modules_graph() const -> ModulesGraph&
{
    return *project().modules_graph;
}
void CommandExecutionContext_Ref::make_sure_node_uses_the_most_up_to_date_version_of_its_definition(Node& node) const
{
    Lab::make_sure_node_uses_the_most_up_to_date_version_of_its_definition(
        node,
        Cool::GetNodeDefinition_Ref<NodeDefinition>{app().nodes_library()},
        modules_graph().graph(),
        command_executor(),
        Cool::GetNodeCategoryConfig_Ref{app().nodes_library()},
        modules_graph().dirty_flags()
    );
}

} // namespace Lab