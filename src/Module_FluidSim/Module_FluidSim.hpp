#pragma once
#include "Cool/Dependencies/SharedVariable.h"
#include "Cool/Gpu/FullscreenPipeline.h"
#include "Cool/Gpu/OpenGL/ComputeShader.h"
#include "Cool/Gpu/OpenGL/SSBO.h"
#include "Cool/TextureSource/TextureSamplerDescriptor.h"
#include "Module/Module.h"
#include "Module/ModuleDependencies.h"

namespace Lab {

struct FluidSimData {
    // Cool::SSBO<glm::vec2> positions{0};
    Cool::SSBO<float> densities{1};

    std::optional<Cool::OpenGL::ComputeShader> init_shader;
    std::optional<Cool::OpenGL::ComputeShader> update_shader;

    Cool::FullscreenPipeline render_shader;

    img::Size grid_size;
};

class Module_FluidSim : public Module {
public:
    Module_FluidSim() = default;
    Module_FluidSim(
        std::string               texture_name_in_shader,
        std::shared_ptr<Module>   module_that_we_depend_on,
        Cool::SharedVariable<int> glitch,
        Cool::SharedVariable<int> custom_resolution
    );

    void init_sim();
    void update_sim(Cool::Time delta_time);

    void on_time_reset() override;

private:
    void render(DataToPassToShader const&) override;

private:
    Cool::SharedVariable<int> _glitch;
    Cool::SharedVariable<int> _custom_resolution;

    std::unique_ptr<FluidSimData> _data;

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
