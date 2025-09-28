#include "NodesCategoryConfig.h"
#include <filesystem>
#include <smart/smart.hpp>
#include "Cool/File/File.h"
#include "Cool/Log/ErrorMessage.hpp"
#include "Cool/Path/Path.h"
#include "Cool/Variables/internal/color_utils.h"
#include "NodeColor.h"
#include "Serialization/SNodesCategoryConfig.h"
#include "imgui.h"

namespace Lab {

void NodesCategoryConfig::save_to_json() const
{
    do_save(*this, _path_to_json);
}

void NodesCategoryConfig::load_from_json()
{
    do_load(*this, _path_to_json)
        .or_else([&](Cool::ErrorMessage const& err) {
            Cool::Log::warning("Nodes Category", fmt::format("Failed to load category config \"{}\":\n{}", Cool::File::weakly_canonical(_path_to_json), err.message), err.clipboard_contents);
        });
}

auto NodesCategoryConfig::imgui_popup() -> bool
{
    bool b = false;

    if (ImGui::BeginPopupContextItem())
    {
        b |= imgui_node_kind_dropdown("Nodes kind", &_nodes_kind);
        b |= ImGui::InputInt("Number of main input pins", &_number_of_main_input_pins);
        _number_of_main_input_pins = smart::keep_above(0, _number_of_main_input_pins);
        ImGui::EndPopup();
    }

    if (b)
        save_to_json();
    return b;
}

} // namespace Lab