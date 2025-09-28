#pragma once
#include "Cool/Gpu/OpenGL/TextureRef.hpp"
#include "Cool/Gpu/RenderTarget.h"
#include "Cool/Log/ErrorMessage.hpp"
#include "Cool/Log/MessageId.h"
#include "Dependencies/Ui.h"
#include "Module/ModuleDependencies.h"
#include "ShaderBased/DataToPassToShader.hpp"
#include "ser20/types/polymorphic.hpp"

namespace Lab {

/// We need to do the polymorphism of Modules through inheritance to allow for plugins to create their own modules.

/// Each module has a State struct, and that's what is serialized / modified through commands / stored in presets.
/// Rendering only has const access to the registries: creating / updating values is done trough ui()

class Module {
public:
    Module()                                     = default; // TODO(Module) remove?
    Module(Module const&)                        = delete;
    auto operator=(Module const&) -> Module&     = delete;
    Module(Module&&) noexcept                    = delete;
    auto operator=(Module&&) noexcept -> Module& = delete;
    virtual ~Module()                            = default;

    explicit Module(std::string_view name, std::string texture_name_in_shader, std::vector<std::shared_ptr<Module>> modules_that_we_depend_on, std::vector<Cool::NodeId> nodes_that_we_depend_on)
        : _name{name}
        , _texture_name_in_shader{std::move(texture_name_in_shader)}
        , _modules_that_we_depend_on{std::move(modules_that_we_depend_on)}
        , _nodes_that_we_depend_on{std::move(nodes_that_we_depend_on)}
    {}

    Cool::NodesGraph const* _nodes_graph{}; // TODO(Particles) Remove

    [[nodiscard]] auto name() const -> const std::string& { return _name; }

    void do_rendering(wgpu::RenderPassEncoder render_pass, DataToPassToShader const& data)
    {
        render(render_pass, data);
        _needs_to_rerender_flag.set_clean();
    }
    virtual void               imgui_windows(Ui_Ref) const {}; /// The ui() method should be const, because it should only trigger commands, not modify internal values (allows us to handle history / re-rendering at a higher level). If you really need to mutate one of your member variables, mark it as `mutable`.
    virtual void               update() {};
    virtual void               on_time_changed() {};
    virtual void               on_time_reset() {};
    virtual void               imgui_generated_shader_code_tab() {};
    [[nodiscard]] virtual auto needs_to_rerender() const -> bool;
    virtual void               before_module_graph_renders() {};
    virtual auto               texture() const -> Cool::TextureRef { return _render_target.texture_ref(); }

    [[nodiscard]] auto needs_to_rerender_flag() const -> Cool::DirtyFlag const& { return _needs_to_rerender_flag; }
    void               update_dependencies_from_nodes_graph(Cool::NodesGraph const& graph) { Lab::update_dependencies_from_nodes_graph(_depends_on, graph, _nodes_that_we_depend_on); }
    void               log_module_error(tl::expected<void, Cool::ErrorMessage> const&, Cool::MessageId&) const;
    auto               render_target() -> Cool::RenderTarget& { return _render_target; }
    auto               render_target() const -> Cool::RenderTarget const& { return _render_target; }
    [[nodiscard]] auto depends_on() const -> ModuleDependencies const& { return _depends_on; }
    [[nodiscard]] auto texture_name_in_shader() const -> std::string const& { return _texture_name_in_shader; }
    [[nodiscard]] auto modules_that_we_depend_on() const -> std::vector<std::shared_ptr<Module>> const& { return _modules_that_we_depend_on; }
    [[nodiscard]] auto nodes_that_we_depend_on() const -> std::vector<Cool::NodeId> const& { return _nodes_that_we_depend_on; }

private:
    virtual void render(DataToPassToShader const&) = 0;

private:
    std::string        _name;
    Cool::DirtyFlag    _needs_to_rerender_flag;
    Cool::RenderTarget _render_target{};

    std::string                          _texture_name_in_shader{};
    std::vector<std::shared_ptr<Module>> _modules_that_we_depend_on{};
    std::vector<Cool::NodeId>            _nodes_that_we_depend_on{};

protected:
    ModuleDependencies _depends_on{};

private:
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Name", _name),
            ser20::make_nvp("Dirty Flag: needs to rerender", _needs_to_rerender_flag)
        );
    }
};

} // namespace Lab