#pragma once
#include "CommandCore/CommandExecutionContext_Ref.h"
#include "SaveAsOptions.hpp"

namespace Lab {

struct Command_SaveProjectAs {
    std::filesystem::path path{};
    SaveAsOptions         options{};

    void               execute(CommandExecutionContext_Ref const& ctx) const;
    [[nodiscard]] auto to_string() const -> std::string;
};

} // namespace Lab

namespace ser20 {
template<class Archive>
void serialize(Archive&, Lab::Command_SaveProjectAs&)
{
}
} // namespace ser20
