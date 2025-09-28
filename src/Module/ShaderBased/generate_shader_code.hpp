#pragma once
#include "DataToGenerateShaderCode.hpp"
#include "Nodes/MaybeGenerateModule.h"

namespace Lab {

using MakeShaderMain = std::function<std::string(std::string const& main_function_name)>;

struct ShaderCodeBits {
    std::string    version;
    std::string    before_main;
    MakeShaderMain make_main;
};

auto generate_shader_code(
    Cool::NodeId const& root_node_id,
    MaybeGenerateModule const&, // TODO(Module) This shouldn't be a parameter, this should be the same for all modules (e.g. a Particles module might also want to generate modules, in exactly the same way that a Compositing module would)
    FunctionSignature const&,
    ShaderCodeBits const&,
    std::function<std::vector<std::string>()> const& get_module_textures_names,
    DataToGenerateShaderCode const&
) -> tl::expected<std::string, std::string>;

} // namespace Lab