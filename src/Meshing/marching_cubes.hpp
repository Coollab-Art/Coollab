#pragma once
#include "Cool/Mesh/Mesh.hpp"
#include "MeshingSettings.hpp"

namespace Lab {

// TODO(Meshing) is there settings that are specific to the marching cubes algorithm?
auto gen_mesh_with_marching_cubes(std::vector<float> const& sdf_samples, MeshingSettings const& settings) -> Cool::Mesh;

} // namespace Lab