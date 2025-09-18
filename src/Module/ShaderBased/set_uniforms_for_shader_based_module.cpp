#include "set_uniforms_for_shader_based_module.hpp"
#include "Cool/Camera/CameraShaderU.h"
#include "Cool/ColorSpaces/ColorAndAlphaSpace.h"
#include "Cool/ColorSpaces/ColorSpace.h"
#include "Cool/Exception/Exception.h"
#include "Cool/Midi/MidiManager.h"
#include "Cool/StrongTypes/set_uniform.h"
#include "Cool/TextureSource/TextureLibrary_Image.h"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "Module/Module.h"
#include "Nodes/Node.h"
#include "Nodes/valid_input_name.h"

namespace Lab {

template<typename T>
static void set_uniform(Cool::OpenGL::Shader const& shader, Cool::SharedVariable<T> const& var, std::string const& node_name)
{
    auto const value = [&] {
        if constexpr (std::is_same_v<T, Cool::Color>)
        {
            auto const col = var.value();
            switch (static_cast<Cool::ColorSpace>(var.get_ref().desired_color_space))
            {
#include "Cool/ColorSpaces/generated/convert_col_as.inl"
            default:
                throw std::runtime_error{fmt::format("Unknown color space value for {}: {}.", var.name(), var.get_ref().desired_color_space)};
            }
        }
        else if constexpr (std::is_same_v<T, Cool::ColorAndAlpha>)
        {
            auto const col = var.value();
            switch (static_cast<Cool::ColorAndAlphaSpace>(var.get_ref().desired_color_space))
            {
#include "Cool/ColorSpaces/generated/convert_col_and_alpha_as.inl"
            default:
                throw std::runtime_error{fmt::format("Unknown color and alpha space value for {}: {}.", var.name(), var.get_ref().desired_color_space)};
            }
        }
        else
        {
            return var.value();
        }
    }();

    try
    {
        Cool::set_uniform(
            shader,
            valid_input_name(var),
            value
        );
        ImGuiNotify::close_immediately(var.notification_id());
    }
    catch (Cool::Exception const& e) // Can be thrown for example when an OSC Channel does not exist
    {
        assert(e.error_message().clipboard_contents.empty() && "Ignoring clipboard contents");
        ImGuiNotify::send_or_change(
            var.notification_id(),
            {
                .type     = ImGuiNotify::Type::Error,
                .title    = "Invalid node parameter",
                .content  = fmt::format("In node \"{}\":\n{}", node_name, e.error_message().message),
                .duration = std::nullopt,
                .closable = false,
            },
            false /*trigger_notification_callbacks_when_changed*/ // To avoid spamming the log file. Otherwise each time we rerender we would re-log the error
        );
    }

    // HACK to send an error message whenever a Texture variable has an invalid path
    if constexpr (std::is_base_of_v<Cool::TextureDescriptor, T>)
    {
        auto const notif = Cool::get_error_notification(value.source);
        if (notif)
        {
            ImGuiNotify::send_or_change(
                var.notification_id(),
                *notif,
                false /*trigger_notification_callbacks_when_changed*/ // To avoid spamming the log file. Otherwise each time we rerender we would re-log the error
            );
        }
        else
        {
            ImGuiNotify::close_immediately(var.notification_id());
        }
    }
}

void set_uniforms_for_shader_based_module(
    Cool::OpenGL::Shader const&                 shader,
    ModuleDependencies const&                   depends_on,
    DataToPassToShader const&                   data,
    std::vector<std::shared_ptr<Module>> const& modules_that_we_depend_on,
    std::vector<Cool::NodeId> const&            nodes_that_we_depend_on
)
{
    shader.bind();
    shader.set_uniform("_camera2D_transform", data.system_values.camera_2D.transform_matrix());
    shader.set_uniform("_camera2D_view", data.system_values.camera_2D.view_matrix());
    shader.set_uniform("_height", data.system_values.height());
    shader.set_uniform("_last_midi_button_pressed", Cool::midi_manager().all_values().last_button_pressed());
    shader.set_uniform("_last_last_midi_button_pressed", Cool::midi_manager().all_values().last_last_button_pressed());
    shader.set_uniform("_time_since_last_midi_button_pressed", Cool::midi_manager().all_values().time_since_last_button_pressed().as_seconds_float());
    shader.set_uniform("_aspect_ratio", data.system_values.aspect_ratio());
    shader.set_uniform("_inverse_aspect_ratio", data.system_values.inverse_aspect_ratio());
    shader.set_uniform_texture("mixbox_lut", Cool::TextureLibrary_Image::instance().get(Cool::Path::root() / "res/mixbox/mixbox_lut.png")->id());
    shader.set_uniform("_time", data.system_values.time.as_seconds_float());
    shader.set_uniform("_delta_time", data.system_values.delta_time.as_seconds_float());

    if (depends_on.audio_volume)
        shader.set_uniform("_audio_volume", data.system_values.audio_manager.get().volume());
    if (depends_on.audio_waveform)
        shader.set_uniform_texture1D("_audio_waveform", data.system_values.audio_manager.get().waveform_texture().id());
    if (depends_on.audio_spectrum)
        shader.set_uniform_texture1D("_audio_spectrum", data.system_values.audio_manager.get().spectrum_texture().id());

    Cool::CameraShaderU::set_uniform(shader, data.system_values.camera_3D, data.system_values.aspect_ratio());

    for (auto const& node_id : nodes_that_we_depend_on)
    {
        data.nodes_graph.nodes().with_ref(node_id, [&](Cool::Node const& abstract_node) {
            auto const& node = abstract_node.downcast<Node>();
            for (auto const& value_input : node.value_inputs())
            {
                std::visit([&](auto&& value_input) {
                    set_uniform(shader, value_input, to_string(node));
                },
                           value_input);
            }
        });
    }

    for (auto const& module : modules_that_we_depend_on)
    {
        shader.set_uniform_texture(
            module->texture_name_in_shader(),
            module->texture().id,
            Cool::TextureSamplerDescriptor{
                .repeat_mode        = Cool::TextureRepeatMode::Clamp,
                .interpolation_mode = glpp::Interpolation::NearestNeighbour, // TODO(FeedbackLoop) The texture coming from feedback loop module must use nearest neighbour interpolation (cf  // Very important. If set to linear, artifacts can appear over time (very visible with the Slit Scan effect).), but the texture from other modules is probably better off using Linear ???
            }
        );
    }
}

} // namespace Lab