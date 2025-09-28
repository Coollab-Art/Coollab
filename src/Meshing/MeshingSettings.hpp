#pragma once

namespace Lab {

struct MeshingSettings {
    glm::vec3  box_size{2.f};     // TODO(Meshing) Clarify : is this the radius or diameter of the box?
    glm::uvec3 samples_count{75}; /// Must always be >= 1 on each axis
    // TODO(Meshing) expose step_size in the GUI, and deduce samples_count from it
    auto step_size() const -> glm::vec3 { return box_size / static_cast<glm::vec3>(samples_count - glm::uvec3{1}); }

    void imgui();

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Box size", box_size),
            ser20::make_nvp("Samples count", samples_count)
        );
    }
};

} // namespace Lab