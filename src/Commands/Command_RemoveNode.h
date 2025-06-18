#pragma once
#include <Cool/Nodes/NodeId.h>
#include "CommandCore/CommandExecutionContext_Ref.h"
#include "CommandCore/MakeReversibleCommandContext_Ref.h"
#include "Nodes/Node.h"
#include "Cool/Serialization/ImGui.hpp" 

namespace Lab {

struct ReversibleCommand_RemoveNode;

struct Command_RemoveNode {
    Cool::NodeId node_id;
    Node         node;

    void execute(CommandExecutionContext_Ref const& ctx) const;
    auto to_string() const -> std::string;
    auto make_reversible(MakeReversibleCommandContext_Ref const& ctx) const -> ReversibleCommand_RemoveNode;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Node id", node_id),
            ser20::make_nvp("Node", node)
        );
    }
};

struct ReversibleCommand_RemoveNode {
    Command_RemoveNode fwd;
    mutable ImVec2     node_pos{};

    void execute(CommandExecutionContext_Ref const& ctx) const;
    void revert(CommandExecutionContext_Ref const& ctx) const;
    auto to_string() const -> std::string;
    auto merge(ReversibleCommand_RemoveNode const&) const -> std::optional<ReversibleCommand_RemoveNode>;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Forward", fwd),
            ser20::make_nvp("Node position", node_pos)
        );
    }
};

} // namespace Lab