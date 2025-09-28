#include "MeshingGui.hpp"
#include <Cool/ImGui/ImGuiExtras.h>
#include <Cool/Path/Path.h>
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/Mesh/export_mesh.hpp"
#include "Nodes/Node.h"
#include "gen_mesh_from_sdf.hpp"

namespace Lab {

MeshingGui::MeshingGui()
    : _window{Cool::icon_fmt("Export a 3D Model", ICOMOON_COGS), Cool::ImGuiWindowConfig{.is_modal = true}}
{
}

void MeshingGui::open_window(Cool::NodeId const& main_node_id)
{
    _main_node_id = main_node_id;
    _window.open();
}

static void gen_and_export_mesh(Cool::NodeId const& main_node_id, MeshingSettings const& meshing_settings, Cool::MeshExportSettings const& mesh_export_settings, DataToPassToShader const& data_to_pass_to_shader, DataToGenerateShaderCode const& data_to_generate_shader_code)
{
    // auto const maybe_node = data_to_generate_shader_code.nodes_graph.try_get_node<Node>(main_node_id);
    // auto const node_def   = data_to_generate_shader_code.get_node_definition(maybe_node->id_names());
    // is_shape_3D(node_def->signature());

    auto const maybe_mesh = gen_mesh_from_sdf(main_node_id, meshing_settings, data_to_pass_to_shader, data_to_generate_shader_code, {}); // TODO(Meshing) We need to generate an entire ModulesGraph, so that we can properly handle the fact that our module might depend on other modules
    if (!maybe_mesh)
        return; // TODO(Meshing) Error message should be handled here
    export_mesh(*maybe_mesh, mesh_export_settings);
}

void MeshingGui::imgui_window(Cool::MeshExportSettings& mesh_export_settings, DataToPassToShader const& data_to_pass_to_shader, DataToGenerateShaderCode const& data_to_generate_shader_code)
{
    _window.show([&](bool is_opening) {
        if (is_opening)
            mesh_export_settings.set_file_name_to_an_unused_name();

        _meshing_settings.imgui();
        ImGui::Separator();
        mesh_export_settings.imgui();

        Cool::ImGuiExtras::before_export_button(mesh_export_settings.path, Cool::PathChecks{});
        if (ImGui::Button(Cool::icon_fmt("Export 3D Model", ICOMOON_UPLOAD2).c_str()))
        {
            gen_and_export_mesh(_main_node_id, _meshing_settings, mesh_export_settings, data_to_pass_to_shader, data_to_generate_shader_code);
            _window.close();
        }
    });
}

} // namespace Lab