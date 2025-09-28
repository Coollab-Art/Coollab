#pragma once
#include "Module/Module.h"
#include "Module/ModuleDependencies.h"

namespace Lab {
class Module_Default : public Module {
public:
    Module_Default() = default;
    Module_Default(std::string texture_name_in_shader);

private:
    void render(DataToPassToShader const&) override;

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
