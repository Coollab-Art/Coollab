#pragma once
#include "Module/ShaderBased/DataToGenerateShaderCode.hpp"
#include "Nodes/MaybeGenerateModule.h"

namespace Lab {

auto generate_compositing_shader_code(
    Cool::NodeId const& root_node_id,
    MaybeGenerateModule const&,
    std::function<std::vector<std::string>()> const& get_module_textures_names,
    DataToGenerateShaderCode const&
) -> tl::expected<std::string, std::string>;

} // namespace Lab