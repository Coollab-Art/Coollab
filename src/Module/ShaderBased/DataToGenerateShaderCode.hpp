#pragma once
#include "Cool/Nodes/GetNodeDefinition_Ref.h"
#include "Cool/Nodes/NodesGraph.h"
#include "Nodes/NodeDefinition.h"

namespace Lab {

struct DataToGenerateShaderCode {
    Cool::NodesGraph const&                     nodes_graph;         // NOLINT(*avoid-const-or-ref-data-members)
    Cool::GetNodeDefinition_Ref<NodeDefinition> get_node_definition; // NOLINT(*avoid-const-or-ref-data-members)
};

} // namespace Lab