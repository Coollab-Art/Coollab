#pragma once
#include "Cool/Mesh/Mesh.hpp"
#include "Cool/Nodes/NodeId.h"
#include "MeshingSettings.hpp"
#include "Module/ShaderBased/DataToGenerateShaderCode.hpp"
#include "Module/ShaderBased/DataToPassToShader.hpp"

namespace Lab {

class Module;

// TODO(Meshing) Return error message in case of failure (tl::expected)
auto gen_mesh_from_sdf(
    Cool::NodeId const& main_node_id,
    MeshingSettings const&,
    DataToPassToShader const&,
    DataToGenerateShaderCode const&,
    std::vector<std::shared_ptr<Module>> const& module_dependencies
) -> std::optional<Cool::Mesh>;

} // namespace Lab