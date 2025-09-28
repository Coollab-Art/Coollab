#pragma once
#include "Cool/Nodes/Editor.h"
#include "Cool/OSC/OSCChannel.h"
#include "Cool/View/GizmoManager.h"
#include "DirtyFlags.h"
#include "Module/Module.h"
#include "Module/ShaderBased/DataToGenerateShaderCode.hpp"
#include "Nodes/NodesConfig.h"

namespace Lab {

/// The main class containing all the nodes of the project.
/// It is responsible for spawning the various modules as required by the nodes, and knowing the dependencies between them.
class ModulesGraph {
public:
    ModulesGraph() = default;

    void update();
    void render(DataToPassToShader const&, DataToGenerateShaderCode const&);

    void request_rerender_all();

    [[nodiscard]] auto is_empty() const -> bool { return _nodes_editor.is_empty(); }
    [[nodiscard]] auto graph() const -> Cool::NodesGraph const& { return _nodes_editor.graph(); }
    [[nodiscard]] auto graph() -> Cool::NodesGraph& { return _nodes_editor.graph(); }
    [[nodiscard]] auto rebuild_modules_graph_flag() const -> Cool::DirtyFlag const& { return _dirty_flags.rebuild; }
    [[nodiscard]] auto rerender_all_flag() const -> Cool::DirtyFlag const& { return _dirty_flags.rerender; }
    [[nodiscard]] auto dirty_flags() const -> DirtyFlags const& { return _dirty_flags; }
    [[nodiscard]] auto nodes_config(Ui_Ref, Cool::AudioManager&, Cool::NodesLibrary const&) const -> NodesConfig;
    void               debug_show_nodes_and_links_registries_windows(Ui_Ref ui) const;
    /// Function called once on every frame where the time has changed.
    void on_time_changed();
    void on_time_reset();
    /// Function called once on every frame where the audio has changed.
    void on_audio_changed();
    /// Function called once on every frame for each OSC channel that has changed.
    void on_osc_channel_changed(Cool::OSCChannel const&);
    /// Function called once on every frame for each Midi channel that has changed.
    void on_midi_channel_changed(Cool::MidiChannel const&);
    void on_last_midi_button_pressed_changed();

    void update_dependencies_from_nodes_graph();

    void imgui_windows(Ui_Ref, Cool::AudioManager&, Cool::NodesLibrary const&) const;
    void submit_gizmos(Cool::GizmoManager&, CommandExecutor const&, Cool::Camera2D const&);

    auto final_texture() const -> Cool::TextureRef;

    //----
    // These functions are mostly here so that Commands can do their job easily
    //----
    auto get_main_node_id() const -> Cool::NodeId const&;
    void set_main_node_id(Cool::NodeId const& id);
    void add_node(Cool::NodeId const&, Node const&);
    void add_link(Cool::LinkId const&, Cool::Link const&);
    void remove_node(Cool::NodeId const&);
    void remove_link(Cool::LinkId const&);
    auto try_get_node(Cool::NodeId const&) const -> Node const*;
    void set_node(Cool::NodeId const&, Node const&);

private:
    void recreate_all_modules(Cool::NodeId const& node_id, DataToGenerateShaderCode const&);
    auto create_module(Cool::NodeId const& node_id, DataToGenerateShaderCode const&) -> std::shared_ptr<Module>;
    auto create_module_impl(std::string const& texture_name_in_shader, std::function<std::shared_ptr<Module>()> const& make_module) -> std::shared_ptr<Module>;
    auto create_compositing_module(Cool::NodeId const& node_id, DataToGenerateShaderCode const&) -> std::shared_ptr<Module>;
    auto create_particles_module(Cool::NodeId const& node_id, NodeDefinition const&, DataToGenerateShaderCode const&) -> std::shared_ptr<Module>;
    auto create_feedback_loop_module(Cool::NodeId const& node_id, DataToGenerateShaderCode const&) -> std::shared_ptr<Module>;
    auto create_default_module() -> std::shared_ptr<Module>;

    void render_module_ifn(Module&, DataToPassToShader const&);
    void check_for_rerender_and_rebuild(DataToPassToShader const&, DataToGenerateShaderCode const&);
    void clear_error_messages();

private:
    mutable Cool::NodesEditor _nodes_editor{};
    mutable Cool::NodeId      _main_node_id{}; // TODO(Modules) Rename as _root_node_id? Or _output_node_id?
    DirtyFlags                _dirty_flags{};

    std::vector<std::shared_ptr<Module>> _modules{};     // TODO(Particles) No need for the unique_ptr (in theory)
    std::shared_ptr<Module>              _root_module{}; // Never null

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Modules", _modules),
            ser20::make_nvp("Dirty flags", _dirty_flags),
            ser20::make_nvp("Node editor", _nodes_editor),
            ser20::make_nvp("Main node ID", _main_node_id)
        );
    }
};

} // namespace Lab
