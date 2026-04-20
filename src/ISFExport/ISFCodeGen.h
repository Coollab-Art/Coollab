#pragma once
#include <Cool/Nodes/GetNodeDefinition_Ref.h>
#include <Cool/Nodes/NodesGraph.h>
#include <filesystem>
#include "ISFMetadata.h"
#include "Nodes/CodeGenContext.h"
#include "Nodes/FunctionSignature.h"
#include "Nodes/NodeDefinition.h"

namespace Lab {

struct ISFCachePass {
    std::string target_name;    // ISF pass TARGET (e.g. "cache_0")
    std::string function_name;  // Function to call when rendering this pass
};

struct ISFCodeGenResult {
    std::string              glsl_code;              // All generated node functions
    std::string              main_function_name;     // Name of the root function to call
    ISFMetadata              metadata;               // ISF JSON metadata
    std::vector<std::string> errors;                 // Unsupported feature errors (abort export)
    std::vector<std::string> warnings;               // Non-fatal warnings (export continues)
    std::vector<std::pair<std::filesystem::path, std::string>> files_to_copy; // source path -> destination filename
    std::vector<ISFCachePass> cache_passes;          // One entry per Caching node → ISF render pass
};

auto isf_generate_code(
    Cool::NodeId const&                         root_node_id,
    FunctionSignature const&                    signature,
    Cool::NodesGraph const&                     graph,
    Cool::GetNodeDefinition_Ref<NodeDefinition> get_node_definition
) -> ISFCodeGenResult;

} // namespace Lab
