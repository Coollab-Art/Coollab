#pragma once
#include "CommandCore/CommandExecutionContext_Ref.h"

namespace Lab {

class App;

struct Command_Log {
    std::string title{};
    std::string content{};

    void execute(CommandExecutionContext_Ref const& ctx) const;
    auto to_string() const -> std::string { return "Logging"; }

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive&)
    {
    }
};

} // namespace Lab