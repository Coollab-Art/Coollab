#pragma once
#include "CommandCore/CommandExecutionContext_Ref.h"
#include "Cool/Websocket/Event.hpp"

namespace Lab {

struct Command_OpenProjectOnNextFrame { // TODO(Commands) Rename as Command_OpenProject
    std::filesystem::path path{};
    std::function<void(Cool::Event)> _callback = nullptr;

    void               execute(CommandExecutionContext_Ref const& ctx) const;
    [[nodiscard]] auto to_string() const -> std::string;
};

} // namespace Lab

namespace ser20 {
template<class Archive>
void serialize(Archive&, Lab::Command_OpenProjectOnNextFrame&)
{
}
} // namespace ser20
