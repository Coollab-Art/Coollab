#pragma once
#include "Cool/ImGui/ImGuiWindow.h"
#include "Cool/Mesh/MeshExportSettings.hpp"
#include "MeshingSettings.hpp"
#include "Module/ShaderBased/DataToGenerateShaderCode.hpp"
#include "Module/ShaderBased/DataToPassToShader.hpp"

namespace Lab {

class MeshingGui {
public:
    MeshingGui();

    void open_window(Cool::NodeId const& main_node_id);
    void imgui_window(Cool::MeshExportSettings&, DataToPassToShader const&, DataToGenerateShaderCode const&);

private:
    Cool::ImGuiWindow _window;
    MeshingSettings   _meshing_settings{};
    Cool::NodeId      _main_node_id{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Settings", _meshing_settings)
        );
    }
};

} // namespace Lab