#pragma once
#include "Cool/Gpu/OpenGL/Shader.h"
#include "DataToPassToShader.hpp"
#include "Module/ModuleDependencies.h"

namespace Lab {

class Module;

void set_uniforms_for_shader_based_module(
    Cool::OpenGL::Shader const&,
    ModuleDependencies const&,
    DataToPassToShader const&,
    std::vector<std::shared_ptr<Module>> const& modules_that_we_depend_on,
    std::vector<Cool::NodeId> const&            nodes_that_we_depend_on
);

} // namespace Lab