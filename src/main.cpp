#include "App.h"
#include "Cool/Path/PathsConfig.h"
//
#include <Cool/Core/run.h> // Must be included last otherwise it slows down compilation because it includes <ser20/archives/json.hpp>

class PathsConfig : public Cool::PathsConfig {
public:
    [[nodiscard]] auto default_texture() const -> std::filesystem::path override
    {
        return Cool::Path::root() / "res/images/logo.png";
    }
};

auto main(int argc, char** argv) -> int
{
    Cool::run<Lab::App, PathsConfig>(
        argc, argv,
        {
            .windows_configs = {{
                .title                  = "", // This is set when we load a project.
                .maximize_on_startup_if = true,
            }},
        }
    );
}