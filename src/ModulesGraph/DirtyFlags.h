#pragma once
#include <Cool/Dependencies/DirtyFlag.h>

namespace Lab {

struct DirtyFlags {
    Cool::DirtyFlag rerender{};
    Cool::DirtyFlag rebuild{};

    /// These two functions are used by variables to know which flag they should use
    auto primary(bool always_requires_shader_code_generation) const -> Cool::DirtyFlag const&;
    auto secondary() const -> Cool::DirtyFlag const&;

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Rerender", rerender),
            ser20::make_nvp("Rebuild", rebuild)
        );
    }
};

} // namespace Lab