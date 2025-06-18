#pragma once
#include "CommandCore/CommandExecutionContext_Ref.h"
#include "img/img.hpp"

namespace Lab {

class App;

struct Command_ExportImage {
    // img::Size size;

    void execute(CommandExecutionContext_Ref const& ctx) const;
    auto to_string() const -> std::string { return "Export image"; }

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive&)
    {
    }
};

} // namespace Lab