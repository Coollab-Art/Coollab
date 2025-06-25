#pragma once
#include "CommandCore/CommandExecutionContext_Ref.h"

namespace Lab {

class App;

struct Command_CloseApp {
    bool force_kill_task_in_progress{false};

    void execute(CommandExecutionContext_Ref const& ctx) const;
    auto to_string() const -> std::string { return "Close app"; }

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive&)
    {
    }
};

} // namespace Lab