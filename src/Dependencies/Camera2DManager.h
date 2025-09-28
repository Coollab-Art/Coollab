#pragma once
#include <Cool/Input/MouseEventDispatcher.h>
#include "Cool/Dependencies/SharedVariable.h"
#include "Cool/StrongTypes/Camera2D.h"
#include "Dependencies/Ui.h"

namespace Lab {

class Camera2DManager {
public:
    Camera2DManager() = default;
    Camera2DManager(std::string variable_name, Cool::DirtyFlag const& dirty_flag)
        : _camera{Cool::Variable<Cool::Camera2D>{{std::move(variable_name)}}, dirty_flag}
    {}

    auto               camera() const -> Cool::Camera2D const& { return _camera.value(); }
    [[nodiscard]] auto is_editable_in_view() -> bool& { return _is_editable_in_view; }

    void imgui(Ui_Ref);
    void hook_events(Cool::MouseEventDispatcher<Cool::ViewCoordinates>& events, CommandExecutor const&);
    void unhook_events(Cool::MouseEventDispatcher<Cool::ViewCoordinates>& events);
    void reset_camera(CommandExecutor const&);

private:
    Cool::SharedVariable<Cool::Camera2D> _camera;
    bool                                 _is_editable_in_view{true};

    reg::AnyId _scroll_callback_id{};
    reg::AnyId _drag_callback_id{};

private:
    // Serialization
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Camera", _camera),
            ser20::make_nvp("Is editable in view", _is_editable_in_view)
        );
    }
};

} // namespace Lab