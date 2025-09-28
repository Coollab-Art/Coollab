#pragma once
#include "Cool/Audio/AudioManager.h"
#include "Cool/Exporter/Exporter.h"
#include "Cool/Image/ImageSizeConstraint.h"
#include "Cool/Image/SharedAspectRatio.hpp"
#include "Cool/Mesh/MeshExportSettings.hpp"
#include "Cool/Midi/MidiManager.h"
#include "Cool/OSC/OSCManager.h"
#include "Cool/Server/ServerManager.hpp"
#include "Cool/Time/Clock_Realtime.h"
#include "Cool/Webcam/WebcamsConfigs.hpp"
#include "Dependencies/Camera2DManager.h"
#include "Dependencies/Camera3DManager.h"
#include "Meshing/MeshingGui.hpp"
#include "ModulesGraph/ModulesGraph.h"
#include "output_view_ptr.hpp"

namespace Lab {

struct Project {
    Project();

    std::unique_ptr<ModulesGraph> modules_graph{std::make_unique<ModulesGraph>()}; // TODO(Modules) Can't we avoid the unique_ptr?
    Camera3DManager               camera_3D_manager{};                             // Must be after modules_graph because we reference a DirtyFlag from it
    Camera2DManager               camera_2D_manager{};                             // Must be after modules_graph because we reference a DirtyFlag from it
    Cool::Clock_Realtime          clock{};
    Cool::ImageSizeConstraint     view_constraint{};
    History                       history{};
    Cool::Exporter                exporter{};
    Cool::AudioManager            audio{};
    Cool::SharedAspectRatio       shared_aspect_ratio{};
    Cool::MeshExportSettings      mesh_export_settings{};
    MeshingGui                    meshing_gui{};

    [[nodiscard]] auto current_clock() const -> Cool::Clock const& { return exporter.is_exporting() ? exporter.clock() : clock; }

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
#if defined(__linux__) || defined(__APPLE__)
        history.set_max_saved_size(0); // TODO HACK to avoid a crash when deserializing the history: https://github.com/orgs/Coollab-Art/projects/1/views/1?pane=issue&itemId=46983814
                                       // This crash is coming from the deserialization of our commands, this is not specific to the history
                                       // We would need to simplify / rewrite our Commands because there is a bug in all our templates and virtual classes
#endif
        archive(
            ser20::make_nvp("Time", clock),
            ser20::make_nvp("View Constraint", view_constraint),
            ser20::make_nvp("Exporter (Image and Video)", exporter),
            ser20::make_nvp("Camera 3D Manager", camera_3D_manager),
            ser20::make_nvp("Camera 2D Manager", camera_2D_manager),
            ser20::make_nvp("Modules Graph", modules_graph),
            ser20::make_nvp("History", history),
            ser20::make_nvp("Audio", audio),
            ser20::make_nvp("Webcams", Cool::WebcamsConfigs::instance()),
            ser20::make_nvp("MIDI", Cool::midi_manager()),
            ser20::make_nvp("OSC", Cool::osc_manager()),
            ser20::make_nvp("Server", Cool::server_manager()),
            ser20::make_nvp("Shared Aspect Ratio", shared_aspect_ratio),
            ser20::make_nvp("3D Model export settings", mesh_export_settings),
            ser20::make_nvp("3D Model generation", meshing_gui),
            ser20::make_nvp("Output view", *output_view_ptr())
        );
    }
};

} // namespace Lab