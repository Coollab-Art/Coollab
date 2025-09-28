#pragma once
#include "Module/Module.h"
#include "Module/ModuleDependencies.h"

namespace Lab {
class Module_FeedbackLoop : public Module {
public:
    Module_FeedbackLoop() = default;
    Module_FeedbackLoop(std::string texture_name_in_shader, std::shared_ptr<Module> module_that_we_depend_on);

    void               on_time_reset() override;
    auto               texture() const -> Cool::TextureRef override;
    [[nodiscard]] auto needs_to_rerender() const -> bool override;
    void               before_module_graph_renders() override;

private:
    void render(DataToPassToShader const&) override;

private:
    Cool::RenderTarget _render_target{};
    bool               _render_target_ping_pong{false};
    int                _renders_count{0};
    bool               _rerender_next_frame{false};
    bool               _rerender_this_frame{false};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Base Module", ser20::base_class<Module>(this))
        );
    }
};

} // namespace Lab
