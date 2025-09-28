#pragma once
#include <Cool/Nodes/NodeId.h>
#include "CommandCore/CommandExecutionContext_Ref.h"

namespace Lab {

struct Command_Meshing {
    Cool::NodeId node_id;

    void execute(CommandExecutionContext_Ref const& ctx) const;

    auto to_string() const -> std::string;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Node id", node_id)
        );
    }
};

} // namespace Lab