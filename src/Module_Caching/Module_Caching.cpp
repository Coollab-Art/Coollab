#include "Module_Caching.hpp"

namespace Lab {

static auto module_id()
{
    static auto i{0};
    return i++;
}

Module_Caching::Module_Caching(std::string texture_name_in_shader, std::shared_ptr<Module> module_that_we_depend_on)
    : Module{
          fmt::format("Caching {}", module_id()),
          std::move(texture_name_in_shader),
          {std::move(module_that_we_depend_on)},
          {} // We don't depend on any node
      }
{}

auto Module_Caching::texture() const -> Cool::TextureRef
{
    // TODO(Module) we don't even need a render target, it should not be part of all modules
    return modules_that_we_depend_on()[0]->texture();
}

void Module_Caching::render(DataToPassToShader const&)
{
}

} // namespace Lab
