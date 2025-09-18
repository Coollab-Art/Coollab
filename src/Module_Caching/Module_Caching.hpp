#pragma once
#include "Module/Module.h"
#include "Module/ModuleDependencies.h"

namespace Lab {

class Module_Caching : public Module {
public:
    Module_Caching() = default;
    Module_Caching(std::string texture_name_in_shader, std::shared_ptr<Module> module_that_we_depend_on);

    auto texture() const -> Cool::TextureRef override;

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
