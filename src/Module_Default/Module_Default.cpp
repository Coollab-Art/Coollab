#include "Module_Default.hpp"

namespace Lab {

static auto module_id()
{
    static auto i{0};
    return i++;
}

Module_Default::Module_Default(std::string texture_name_in_shader)
    : Module{
          fmt::format("Default {}", module_id()),
          std::move(texture_name_in_shader),
          {}, // We don't depend on any module
          {}  // We don't depend on any node
      }
{}

void Module_Default::render(DataToPassToShader const& data)
{
    render_target().set_size(data.system_values.render_target_size);
    render_target().render([&]() {
        glClearColor(0.f, 0.f, 0.f, 0.f);
        glClear(GL_COLOR_BUFFER_BIT);
    });
}

} // namespace Lab
