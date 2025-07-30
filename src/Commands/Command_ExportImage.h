#pragma once
#include "CommandCore/CommandExecutionContext_Ref.h"
#include "img/img.hpp"
#include "Cool/Websocket/Event.hpp"

namespace Lab {

class App;

struct Command_ExportImage {
    img::ImageExportParams image_export_params{};
    std::function<void(Cool::Event)> _start_callback = nullptr;
    std::function<void(Cool::Event)> _end_callback = nullptr;

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