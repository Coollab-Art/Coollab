#pragma once
#include "CommandCore/CommandExecutionContext_Ref.h"

namespace Lab {

struct Command_OpenProjectOnNextFrame {
    std::filesystem::path path{};

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
