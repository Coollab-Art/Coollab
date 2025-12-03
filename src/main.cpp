#include "App.h"
#include "Cool/Dump/set_extra_dump_info.hpp"
#include "Cool/Path/PathsConfig.h"
//
#include <Cool/Core/run.h> // Must be included last otherwise it slows down compilation because it includes <ser20/archives/json.hpp>

class PathsConfig : public Cool::PathsConfig {
public:
    [[nodiscard]] auto default_texture() const -> std::filesystem::path override
    {
        return Cool::Path::root() / "res/images/logo.png";
    }
    [[nodiscard]] auto default_video() const -> std::filesystem::path override
    {
        return Cool::Path::root() / "res/images/logo.gif";
    }
};

auto main(int argc, char** argv) -> int
{
    Cool::set_extra_dump_info([](Cool::DumpStringGenerator& dump) {
        dump.add("OpenSSL",
#if CPPHTTPLIB_OPENSSL_SUPPORT
                 "Enabled"
#else
                 "Disabled"
#endif
        );
    });
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