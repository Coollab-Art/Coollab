#include "Camera2DManager.h"
#include <Commands/Command_SetVariable.h>
#include <Cool/Camera/ViewController_OrbitalU.h>
#include <Dependencies/Ui.h>
#include <glm/gtx/matrix_transform_2d.hpp>
#include "Cool/Input/MouseCoordinates.h"
#include "Cool/Input/MouseDragEvents.h"
#include "Cool/StrongTypes/Camera2D.h"
#include "Cool/UserSettings/UserSettings.h"
#include "Cool/Utils/Chrono.h"
#include "glm/gtx/rotate_vector.hpp"
#include "imgui.h"

namespace Lab {

void Camera2DManager::hook_events(Cool::MouseEventDispatcher<Cool::ViewCoordinates>& events, CommandExecutor const& executor)
{
    _scroll_callback_id =
        events
            .scroll_event()
            .subscribe([&, executor](Cool::MouseScrollEvent<Cool::ViewCoordinates> const& event) {
                if (!_is_editable_in_view)
                    return;

                float const sensitivity    = Cool::user_settings().camera2D_zoom_sensitivity;
                float const zoom_variation = std::pow(sensitivity, event.dy);

                auto new_value = _camera.value();
                if (!ImGui::GetIO().KeyAlt) // Use ALT to zoom relative to the center of the view
                {
                    auto const mouse_pos_in_view_space  = event.position;
                    auto const mouse_pos_in_world_space = glm::vec2{new_value.transform_matrix() * glm::vec3{mouse_pos_in_view_space, 1.f}};
                    auto const rotated_mouse_in_ws      = glm::rotate(mouse_pos_in_world_space, -new_value.rotation.as_radians());

                    new_value.translation = new_value.translation / zoom_variation + rotated_mouse_in_ws * (1.f - 1.f / zoom_variation);
                }
                new_value.zoom *= zoom_variation;

                { // Don't merge with zooms that happened a while ago
                    static auto chrono = Cool::Chrono{};
                    if (chrono.elapsed_more_than(0.5s))
                        executor.execute(Command_FinishedEditingVariable{});
                }
                executor.execute(Command_SetVariable<Cool::Camera2D>{.var_ref = _camera.get_ref(), .value = new_value});
            });

    _drag_callback_id =
        events
            .drag()
            .subscribe({
                .on_start  = [&](auto&&) { return _is_editable_in_view; },
                .on_update = [&, executor](Cool::MouseDragUpdateEvent<Cool::ViewCoordinates> const& event) {
                    auto new_value = _camera.value();
                    new_value.translation -= event.delta / _camera.value().zoom;
                    executor.execute(Command_SetVariable<Cool::Camera2D>{.var_ref = _camera.get_ref(), .value = new_value});

                    ImGui::WrapMousePos(ImGuiAxesMask_All);
                    //
                },
                .on_stop = [&, executor](auto&&) { executor.execute(Command_FinishedEditingVariable{}); },
            });

    // TODO(Camera2D) rotation when shift+scroll and explanation
}

void Camera2DManager::unhook_events(Cool::MouseEventDispatcher<Cool::ViewCoordinates>& events)
{
    events.scroll_event().unsubscribe(_scroll_callback_id);
    events.drag().unsubscribe(_drag_callback_id);
}

void Camera2DManager::reset_camera(CommandExecutor const& executor)
{
    executor.execute(Command_SetVariable<Cool::Camera2D>{.var_ref = _camera.get_ref(), .value = {}});
    executor.execute(Command_FinishedEditingVariable{});
}

void Camera2DManager::imgui(Ui_Ref ui)
{
    ui.widget(_camera);
}

} // namespace Lab