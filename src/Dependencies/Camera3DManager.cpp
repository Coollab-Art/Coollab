#include "Camera3DManager.h"
#include "CommandCore/CommandExecutor.h"
#include "Commands/Command_FinishedEditingVariable.h"
#include "Commands/Command_SetCameraZoom.h"
#include "Commands/Command_SetVariable.h"
#include "Cool/Camera/ViewController_OrbitalU.h"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/Utils/Chrono.h"
#include "glm/gtx/euler_angles.hpp"
#include "glm/gtx/matrix_decompose.hpp"

namespace Lab {

void Camera3DManager::hook_events(
    Cool::MouseEventDispatcher<Cool::ViewCoordinates>& events,
    CommandExecutor const&                             executor
)
{
    _scroll_callback_id =
        events
            .scroll_event()
            .subscribe([this, executor](auto const& event) {
                if (!_is_editable_in_view)
                    return;

                auto       camera   = _camera.value();
                auto const old_zoom = _view_controller.get_distance_to_orbit_center();
                if (_view_controller.on_wheel_scroll(camera, event.dy))
                {
                    auto const zoom = _view_controller.get_distance_to_orbit_center();
                    _view_controller.set_distance_to_orbit_center(old_zoom); // Undo the zoom, it will be done by the Command_SetCameraZoom

                    { // Don't merge with zooms that happened a while ago
                        static auto chrono = Cool::Chrono{};
                        if (chrono.elapsed_more_than(0.5s))
                            executor.execute(Command_FinishedEditingVariable{});
                    }
                    executor.execute(Command_SetCameraZoom{zoom});
                }
            });
    _drag_callback_id =
        events
            .drag()
            .subscribe({
                .on_start  = [this, executor](auto const&) {
                if (!_is_editable_in_view)
                    return false;

                maybe_update_camera( executor,  [&](Cool::Camera& camera) {
                    return _view_controller.on_drag_start(camera);
                });
                return true; },
                .on_update = [this, executor](auto const&) {
                    //
                    maybe_update_camera(executor, [&](Cool::Camera& camera) {
                        return _view_controller.on_drag(camera, ImGui::GetIO().MouseDelta); // NB: we don't use event.delta as it is in relative coordinates, and we want a delta in pixels to keep the drag speed the same no matter the size of the View.
                    });
                    ImGui::WrapMousePos(ImGuiAxesMask_All);
                    //
                },
                .on_stop = [this, executor](auto&&) {
                maybe_update_camera( executor,  [&](Cool::Camera& camera) {
                    return _view_controller.on_drag_stop(camera);
                });
                executor.execute(Command_FinishedEditingVariable{}); },
            });
}

void Camera3DManager::unhook_events(Cool::MouseEventDispatcher<Cool::ViewCoordinates>& events
)
{
    events.scroll_event().unsubscribe(_scroll_callback_id);
    events.drag().unsubscribe(_drag_callback_id);
}

void Camera3DManager::imgui(CommandExecutor const& executor)
{
    maybe_update_camera(executor, [&](Cool::Camera& camera) {
        bool b = false;

        glm::vec3 scale;
        glm::quat rotation;
        glm::vec3 translation;
        glm::vec3 skew;
        glm::vec4 perspective;

        glm::decompose(camera.view_matrix(), scale, rotation, translation, skew, perspective);

        glm::vec3 eulerAnglesRadians = glm::eulerAngles(rotation);
        b |= ImGui::DragFloat3("Position", glm::value_ptr(translation));
        b |= ImGui::DragFloat3("Rotation", glm::value_ptr(eulerAnglesRadians));

        if (b)
        {
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
            glm::mat4 rotationMatrix    = glm::eulerAngleXYZ(eulerAnglesRadians.x, eulerAnglesRadians.y, eulerAnglesRadians.z);
            glm::mat4 scaleMatrix       = glm::scale(glm::mat4(1.0f), scale);
            camera.set_view_matrix(translationMatrix * rotationMatrix * scaleMatrix);
        }
        return b;
    });
    maybe_update_camera(executor, [&](Cool::Camera& camera) {
        return _view_controller.ImGui(camera);
    });
    maybe_update_camera(executor, [&](Cool::Camera& camera) {
        return Cool::imgui(camera.projection());
    });
    if (ImGui::IsItemDeactivatedAfterEdit())
        executor.execute(Command_FinishedEditingVariable{});
    if (ImGui::Button(Cool::icon_fmt("Look at the origin", ICOMOON_RADIO_CHECKED).c_str()))
    {
        maybe_update_camera(executor, [&](Cool::Camera& camera) {
            _view_controller.set_orbit_center({0, 0, 0}, camera);
            return true;
        });
        executor.execute(Command_FinishedEditingVariable{});
    }
    if (ImGui::Button(Cool::icon_fmt("Reset Camera", ICOMOON_TARGET).c_str()))
    {
        reset_camera(executor);
    }
}

void Camera3DManager::maybe_update_camera(
    CommandExecutor const&                    executor,
    std::function<bool(Cool::Camera&)> const& fun
)
{
    auto new_value = _camera.value();
    if (fun(new_value))
        executor.execute(Command_SetVariable<Cool::Camera>{_camera.get_ref(), new_value});
}

void Camera3DManager::set_zoom(float zoom)
{
    Cool::ViewController_OrbitalU::set_distance_to_orbit_center(_view_controller, _camera.value(), zoom);
    _camera.dirty_flag().set_dirty();
}

void Camera3DManager::reset_camera(CommandExecutor const& executor)
{
    maybe_update_camera(executor, [&](Cool::Camera& camera) {
        Cool::ViewController_OrbitalU::reset_transform(_view_controller, camera);
        return true;
    });
    executor.execute(Command_FinishedEditingVariable{});
}

} // namespace Lab
