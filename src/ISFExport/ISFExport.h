#pragma once
#include <Cool/Nodes/GetNodeDefinition_Ref.h>
#include <Cool/Nodes/NodesGraph.h>
#include <filesystem>
#include "Nodes/NodeDefinition.h"
#include "tl/expected.hpp"

namespace Lab {

struct Project;

struct ISFExportParams {
    Cool::NodesGraph const&                     graph;
    Cool::NodeId const&                         root_node_id;
    Cool::GetNodeDefinition_Ref<NodeDefinition> get_node_definition;
    glm::mat3                                   camera_2D_transform;  // Baked camera2D transform (aspect-ratio-independent)
    glm::mat3                                   camera_2D_view;       // Baked camera2D view (aspect-ratio-independent)
    glm::mat4                                   camera_3D_view;       // Baked camera3D view matrix (aspect-ratio-independent)
    float                                       camera_3D_fov;        // Field of view in radians
    float                                       camera_3D_near_plane;
    float                                       camera_3D_far_plane;
};

struct ISFExportResult {
    std::vector<std::string> warnings;
};

auto export_as_isf(
    ISFExportParams const&          params,
    std::filesystem::path const&    output_path
) -> tl::expected<ISFExportResult, std::string>;

} // namespace Lab
