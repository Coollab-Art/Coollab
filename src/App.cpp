#include "App.h"
#include <filesystem>
#include "CommandCore/command_to_string.h"
#include "CommandCore/get_app.hpp"
#include "Commands/Command_OpenImageExporter.h"
#include "Commands/Command_OpenVideoExporter.h"
#include "Cool/DebugOptions/debug_options_windows.h"
#include "Cool/File/File.h"
#include "Cool/File/PathChecks.hpp"
#include "Cool/ImGui/ColorThemes.h"
#include "Cool/ImGui/Fonts.h"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/ImGuiExtrasStyle.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/ImGui/markdown.h"
#include "Cool/Image/SaveImage.h"
#include "Cool/Input/CTRL_OR_CMD.hpp"
#include "Cool/Log/message_console.hpp"
#include "Cool/OSC/OSCChannel.h"
#include "Cool/OSC/OSCManager.h"
#include "Cool/Path/Path.h"
#include "Cool/Server/ServerManager.hpp"
#include "Cool/Time/ClockU.h"
#include "Cool/Tips/TipsManager.h"
#include "Cool/UserSettings/UserSettings.h"
#include "Cool/Video/hack_get_global_time_in_seconds.h"
#include "Cool/View/View.h"
#include "Cool/View/ViewsManager.h"
#include "Cool/Webcam/WebcamsConfigs.hpp"
#include "Debug/DebugOptions.h"
#include "Dependencies/Camera2DManager.h"
#include "ModulesGraph/ModulesGraph.h"
#include "ProjectManager/COOLLAB_FILE_EXTENSION.hpp"
#include "ProjectManager/Command_OpenProjectOnNextFrame.hpp"
#include "ProjectManager/Command_SaveProject.h"
#include "ProjectManager/Command_SaveProjectAs.h"
#include "ProjectManager/Interfaces.hpp"
#include "ProjectManager/ProjectManager.hpp"
#include "Tips/Tips.h"
#include "UserSettings/UserSettings.hpp"
#include "open/open.hpp"

namespace Lab {

App::App(Cool::WindowManager& windows, Cool::ViewsManager& views)
    : _main_window{windows.main_window()}
    , _output_view{views.make_view<Cool::TextureView>(Cool::ViewCreationParams{
          .name           = Cool::icon_fmt("Output", ICOMOON_IMAGE),
          .is_output_view = true,
          .is_closable    = true,
          .start_open     = false,
      })}
    , _preview_view{views.make_view<Cool::ForwardingOrTextureView>(
          _output_view,
          Cool::ViewCreationParams{
              .name        = Cool::icon_fmt("View", ICOMOON_IMAGE),
              .is_closable = false,
              .start_open  = true,
          }
      )}
{
    internal::get_app() = this;
    output_view_ptr()   = &_output_view;
}

void App::init()
{
    _project_manager.process_command_line_args(make_on_project_loaded(), make_on_project_unloaded(), make_window_title_setter());
}

void App::update()
{
    // Must be done first in the frame, because then we use the current project, and don't want to change it during the frame
    _project_manager.open_requested_project_if_any(make_on_project_loaded(), make_on_project_unloaded(), make_window_title_setter());

    project().history.start_new_commands_group(); // All commands done in one frame are grouped together, and will be done / undone at once.

    user_settings().update();

    if (project().shared_aspect_ratio.fill_the_view)
        project().shared_aspect_ratio.aspect_ratio.set(render_view().aspect_ratio());

    if (DebugOptions::force_rerender_every_frame())
        project().modules_graph->request_rerender_all();

    project().audio.set_force_mute(project().exporter.is_exporting());
    project().audio.sync_with_clock(
        project().current_clock(),
        project().exporter.is_exporting() /* force_sync_time */
    );
    Cool::hack_get_global_time_in_seconds() = project().current_clock().time();
    Cool::hack_get_is_exporting()           = project().exporter.is_exporting();
    project().audio.update(/*on_audio_data_changed = */ [&]() {
        project().modules_graph->on_audio_changed();
    });

    project().modules_graph->update_dependencies_from_nodes_graph(); // TODO(Modules) Don't recompute dependencies on every frame. Instead we should probably store a ref to the variables that use OSC or Midi, so that we can check each time to see which channel they are currently using.
    Cool::osc_manager().for_each_channel_that_has_changed([&](Cool::OSCChannel const& osc_channel) {
        project().modules_graph->on_osc_channel_changed(osc_channel);
    });
    Cool::midi_manager().for_each_channel_that_has_changed([&](Cool::MidiChannel const& midi_channel) {
        project().modules_graph->on_midi_channel_changed(midi_channel);
    });
    if (Cool::midi_manager().last_button_pressed_has_changed())
        project().modules_graph->on_last_midi_button_pressed_changed();

    if (inputs_are_allowed()) // Must update() before we render() to make sure the modules are ready (e.g. Nodes need to parse the definitions of the nodes from files)
    {
        _nodes_library_manager.update(project().modules_graph->rebuild_modules_graph_flag(), project().modules_graph->graph(), project().modules_graph->nodes_config(ui(), project().audio, _nodes_library_manager.library()));
        project().modules_graph->update();
        check_inputs();
    }

    if (!project().exporter.is_exporting())
    {
        project().clock.update();
        auto const render_size = spout_out_manager().is_active()
                                     ? spout_out_manager().texture_size()
                                     : render_view().desired_image_size(project().view_constraint); // TODO(JF) Integrate the notion of View Constraint inside the TextureView ? But that's may be too much coupling
        polaroid().render(render_size, project().clock.time(), project().clock.delta_time());
    }
    else
    {
        request_rerender();
        project().exporter.update(polaroid());
    }

    // if (_custom_shader_view.render_target.needs_resizing())
    // {
    // set_dirty_flag()(_custom_shader_module->dirty_flag());
    // }

    DebugOptionsManager::update();
}

void App::save_project_thumbnail()
{
    if (!_project_manager.has_registered_project_to_the_launcher())
        return;
    auto const info_folder_for_the_launcher = _project_manager.info_folder_for_the_launcher();
    if (!info_folder_for_the_launcher.has_value())
        return;

    save_project_thumbnail_impl(*info_folder_for_the_launcher);
}

void App::save_project_thumbnail_impl(std::filesystem::path const& folder_path)
{
    auto const polar = polaroid();
    polar.render({100, 100}, project().clock.time(), project().clock.delta_time());
    Cool::ImageU::save(folder_path / "thumbnail.png", polar.texture().download_pixels())
        .or_else([&](std::string const& err) {
            Cool::Log::internal_warning("Save Thumbnail", err);
        });
}

void App::on_shutdown()
{
    command_execution_context().execute(Command_SaveProject{.is_autosave = true, .must_absolutely_succeed = true});
    on_project_unloaded();
    user_settings().save();
    _tips_manager.on_app_shutdown();
    _is_shutting_down = true;
    DebugOptions::save();
    _project_manager.on_shutdown();
}

void App::on_project_loaded()
{
    project().camera_3D_manager.hook_events(_preview_view.mouse_events(), command_executor());
    project().camera_2D_manager.hook_events(_preview_view.mouse_events(), command_executor());

    project().view_constraint.set_shared_aspect_ratio(project().shared_aspect_ratio);
    project().exporter.set_shared_aspect_ratio(project().shared_aspect_ratio);
    _gallery_publisher.set_shared_aspect_ratio(project().shared_aspect_ratio);
    spout_out_manager().set_shared_aspect_ratio(project().shared_aspect_ratio);

    project().spout_out_manager.activate_ifn();

    auto const ctx = command_execution_context();
    for (auto& [_, node] : project().modules_graph->graph().nodes())
        ctx.make_sure_node_uses_the_most_up_to_date_version_of_its_definition(node.downcast<Node>());
}

void App::on_project_unloaded()
{
    save_project_thumbnail();

    project().camera_3D_manager.unhook_events(_preview_view.mouse_events());
    project().camera_2D_manager.unhook_events(_preview_view.mouse_events());
}

void App::compile_all_is0_nodes()
{
    // for (auto const& node_template : project().modules_graph->compositing_module().nodes_templates())
    // {
    //     project().modules_graph->compositing_module().remove_all_nodes();
    //     Cool::Log::ToUser::info("Test is0 Node", node_template.name);
    //     project().modules_graph->compositing_module().add_node(NodeFactoryU::node_from_template(node_template));
    //     project().modules_graph->compositing_module().recompile(update_context(), true);
    // }
    // project().modules_graph->compositing_module().remove_all_nodes();
}

auto App::make_window_title_setter() -> SetWindowTitle
{
    return [window_ptr = _main_window.glfw()](std::string_view title) {
        glfwSetWindowTitle(window_ptr, title.data());
    };
}

auto App::make_on_project_loaded() -> OnProjectLoaded
{
    return [&]() {
        on_project_loaded();
    };
}

auto App::make_on_project_unloaded() -> OnProjectUnloaded
{
    return [&]() {
        on_project_unloaded();
    };
}

auto App::make_save_thumbnail() -> SaveThumbnail
{
    return [&](std::filesystem::path const& folder_path) {
        save_project_thumbnail_impl(folder_path);
    };
}

void App::on_time_changed()
{
    project().modules_graph->on_time_changed();
}

void App::on_time_reset()
{
    project().modules_graph->on_time_reset();
}

void App::request_rerender() // TODO(Modules) Sometimes we don't need to call this, but only rerender a specific module instead
{
    project().modules_graph->request_rerender_all();
}

auto App::render_view() -> Cool::TextureView&
{
    if (_output_view.is_open())
        return _output_view;
    return _preview_view;
}

Cool::Polaroid App::polaroid()
{
    return {
        .texture = [this]() { return project().modules_graph->final_texture(); },
        .render  = [this](img::Size size, Cool::Time time, Cool::Time delta_time) { render(size, time, delta_time); },
    };
}

auto App::inputs_are_allowed() const -> bool
{
    return !project().exporter.is_exporting();
}

auto App::wants_to_show_menu_bar() const -> bool
{
    return !project().exporter.is_exporting() && !_wants_view_in_fullscreen;
}

static void imgui_window_console()
{
    Cool::message_console().imgui_window();
}

void App::render(img::Size size, Cool::Time time, Cool::Time delta_time)
{
    if (_project_manager.is_in_safe_mode())
        return;

    if (_last_time != time)
    {
        _last_time = time;
        on_time_changed();
    }
    project().modules_graph->render(
        data_to_pass_to_shader(size, time, delta_time),
        data_to_generate_shader_code()
    );

    if (spout_out_manager().is_active())
        spout_out_manager().send_texture(project().modules_graph->final_texture());
}

void App::imgui_window_cameras()
{
    if (ImGui::BeginTabBar("##cameras"))
    {
        ImGui::PushFont(Cool::Font::bold());
        if (ImGui::BeginTabItem(Cool::icon_fmt("2D Camera", ICOMOON_CAMERA).c_str(), nullptr, project().camera_2D_manager.is_editable_in_view() ? ImGuiTabItemFlags_SetSelected : 0))
        {
            ImGui::PopFont();
            if (ImGui::IsItemActive())
            {
                project().camera_2D_manager.is_editable_in_view() = true;
                project().camera_3D_manager.is_editable_in_view() = false;
            }
            project().camera_2D_manager.imgui(ui());
            ImGui::EndTabItem();
        }
        else
        {
            ImGui::PopFont();
        }

        ImGui::PushFont(Cool::Font::bold());
        if (ImGui::BeginTabItem(Cool::icon_fmt("3D Camera", ICOMOON_VIDEO_CAMERA).c_str(), nullptr, project().camera_3D_manager.is_editable_in_view() ? ImGuiTabItemFlags_SetSelected : 0))
        {
            ImGui::PopFont();
            if (ImGui::IsItemActive())
            {
                project().camera_3D_manager.is_editable_in_view() = true;
                project().camera_2D_manager.is_editable_in_view() = false;
            }
            project().camera_3D_manager.imgui(command_executor());
            ImGui::PopID();
        }
        else
        {
            ImGui::PopFont();
        }
        ImGui::EndTabBar();
    }
}

void App::imgui_window_view()
{
    if (_project_manager.is_in_safe_mode())
    {
        ImGui::Begin(Cool::icon_fmt("Crash", ICOMOON_LIFEBUOY).c_str());
        Cool::ImGuiExtras::warning_text("Safe Mode");
        Cool::ImGuiExtras::markdown("Rendering is disabled because the project crashed last time. Undo your recent changes to the nodes patch, and once you think it shouldn't crash anymore press \"Go!\"");
        if (ImGui::Button("Go!"))
            _project_manager.exit_safe_mode();
        ImGui::End();
    }

    bool const view_in_fullscreen = project().exporter.is_exporting() || _wants_view_in_fullscreen;
    {
        if (!_view_was_in_fullscreen_last_frame && view_in_fullscreen)
            save_imgui_windows_state(); // Save normal state before making the View fullscreen.
        if (_view_was_in_fullscreen_last_frame && !view_in_fullscreen)
            restore_imgui_windows_state(); // After fullscreen, restore the normal state.
        _view_was_in_fullscreen_last_frame = view_in_fullscreen;
    }

    project().modules_graph->submit_gizmos(_preview_view.gizmos_manager(), command_executor(), project().camera_2D_manager.camera());
    _output_view.set_texture(project().modules_graph->final_texture());
    _output_view.imgui_window({
        // When we switch between using the _output_view and the _nodes_view we need to rerender on the new render target.
        .on_open  = [&]() { request_rerender(); _disable_sleep.emplace("Coollab", "Coollab \"Output\" window is open", no_sleep::Mode::KeepScreenOnAndKeepComputing); },
        .on_close = [&]() { request_rerender(); _disable_sleep.reset(); },
    });
    _preview_view.set_texture(project().modules_graph->final_texture());
    _preview_view.imgui_window({
        .fullscreen    = view_in_fullscreen,
        .extra_widgets = [&]() {
            if (project().exporter.is_exporting())
                return false;
            bool b = false;

            bool const align_buttons_vertically = _preview_view.has_vertical_margins()
                                                  || (!project().view_constraint.does_constrain_aspect_ratio() && !_output_view.is_open()); // Hack to avoid flickering the alignment of the buttons when we are resizing the View

            int buttons_order{0};
            // Reset cameras
            if (Cool::ImGuiExtras::floating_button(ICOMOON_TARGET, buttons_order++, align_buttons_vertically))
            {
                reset_cameras();
            }
            b |= ImGui::IsItemActive();
            ImGui::SetItemTooltip("%s", "Reset 2D and 3D cameras");

            // Toggle fullscreen
            if (Cool::ImGuiExtras::floating_button(_wants_view_in_fullscreen ? ICOMOON_SHRINK : ICOMOON_ENLARGE, buttons_order++, align_buttons_vertically))
            {
                _wants_view_in_fullscreen = !_wants_view_in_fullscreen;
                _main_window.set_fullscreen(_wants_view_in_fullscreen);
            }
            b |= ImGui::IsItemActive();
            ImGui::SetItemTooltip("%s", _wants_view_in_fullscreen ? "Shrink the view" : "Expand the view");

            // Toggle 2D / 3D cameras
            if (Cool::ImGuiExtras::floating_button(project().camera_2D_manager.is_editable_in_view() ? ICOMOON_CAMERA : ICOMOON_VIDEO_CAMERA, buttons_order++, align_buttons_vertically))
            {
                project().camera_2D_manager.is_editable_in_view() = !project().camera_2D_manager.is_editable_in_view();
                project().camera_3D_manager.is_editable_in_view() = !project().camera_2D_manager.is_editable_in_view(); // Only allow one camera active at the same time.
            }
            b |= ImGui::IsItemActive();
            ImGui::SetItemTooltip("%s", project().camera_2D_manager.is_editable_in_view() ? "2D camera is active" : "3D camera is active");

            // Aspect Ratio
            if (Cool::ImGuiExtras::floating_button(ICOMOON_CROP, buttons_order++, align_buttons_vertically))
                ImGui::OpenPopup("##Aspect Ratio");
            ImGui::SetItemTooltip("%s", "Aspect Ratio");
            if (ImGui::BeginPopup("##Aspect Ratio"))
            {
                project().view_constraint.imgui_aspect_ratio();
                ImGui::EndPopup();
            }
            b |= ImGui::IsItemActive();

            return b;
        },
    });
}

static auto file_to_save_backup_project(std::filesystem::path const& image_path)
{
    return Cool::File::with_extension(image_path, COOLLAB_FILE_EXTENSION);
}

static auto image_export_path_checks() -> Cool::PathChecks
{
    return Cool::PathChecks{
        .warnings_checks = {
            Cool::export_path_existence_check(),
            [](std::filesystem::path const& image_path) {
                auto const backup_project_path = file_to_save_backup_project(image_path);
                return Cool::File::exists(backup_project_path)
                           // Make sure that if a project file already exists at "img(3).coollab" we won't try to call the image "img(3).png" even if that png file doesn't exist. We will skip directly to "img(4).png", to make sure we are able to create a project with the same name, without conflicting with any existing project.
                           ? fmt::format("A backup project \"{}\" already exists. Are you sure you want to overwrite it?", Cool::File::file_name(backup_project_path))
                           : ""s;
            },
        },
    };
}

void App::on_image_export_start(std::filesystem::path const& exported_image_path)
{
    command_executor().execute(Command_SaveProjectAs{
        file_to_save_backup_project(exported_image_path),
        {
            .register_project_in_the_launcher = false,
            .allow_overwrite_existing_file    = true,
        },
    });
}

void App::imgui_window_exporter()
{
    project().exporter.imgui_windows({
        .polaroid                                   = polaroid(),
        .time                                       = project().clock.time(),
        .delta_time                                 = project().clock.delta_time(),
        .time_speed                                 = project().clock.time_speed().value(),
        .on_image_export_start                      = [&](std::filesystem::path const& exported_image_path) { on_image_export_start(exported_image_path); },
        .on_video_export_start                      = [&]() { on_time_reset(); },
        .widgets_in_window_video_export_in_progress = [&]() {
            ImGui::NewLine();
            ImGui::SeparatorText(Cool::icon_fmt("Did you know?", ICOMOON_BUBBLE).c_str());
            _tips_manager.imgui_show_one_tip(all_tips());
            //
        },
        .image_path_checks = image_export_path_checks(),
    });
}

void App::imgui_window_meshing()
{
    project().meshing_gui.imgui_window(
        project().mesh_export_settings,
        data_to_pass_to_shader(render_view().desired_image_size(project().view_constraint), project().clock.time(), project().clock.delta_time()),
        data_to_generate_shader_code()
    );
}

void App::imgui_window_license()
{
    if (_license_window_is_open)
    {
        ImGui::Begin(Cool::icon_fmt("License", ICOMOON_FILE_TEXT).c_str(), &_license_window_is_open);
        static auto license_text = Cool::File::to_string(Cool::Path::root() / "LICENSE.txt");
        if (license_text.has_value())
            ImGui::TextUnformatted(license_text->c_str());
        ImGui::End();
    }
}

void App::imgui_windows()
{
    imgui_window_view();
    imgui_window_exporter();
    imgui_window_console();
    spout_out_manager().imgui_window();
    if (inputs_are_allowed())
        imgui_windows_only_when_inputs_are_allowed();
}

void App::imgui_windows_only_when_inputs_are_allowed()
{
    imgui_window_meshing();
    imgui_window_license();

    auto const the_ui = ui();
    // Time
    ImGui::Begin(Cool::icon_fmt("Time", ICOMOON_STOPWATCH).c_str());
    Cool::ClockU::imgui_timeline(
        project().clock,
        /* extra_widgets = */ [&]() {
            ImGui::SetNextItemWidth(3.5f * ImGui::GetFontSize());
            the_ui.widget(project().clock.time_speed());
            //
        },
        /* on_time_reset = */ [&]() { on_time_reset(); }
    );
    ImGui::End();
    // Audio
    project().audio.imgui_window();
    // Webcams
    Cool::WebcamsConfigs::instance().imgui_window();
    // Midi
    Cool::midi_manager().imgui_window();
    // OSC
    Cool::osc_manager().imgui_window();
    // Server
    Cool::server_manager().imgui_window();
    // Tips
    _tips_manager.imgui_windows(all_tips());
    // Nodes
    project().modules_graph->imgui_windows(the_ui, project().audio, _nodes_library_manager.library());
    // Share online
    _gallery_publisher.imgui_window([&](img::Size size) {
        auto the_polaroid = polaroid();
        the_polaroid.render(size, project().clock.time(), project().clock.delta_time());
        return the_polaroid.texture().download_pixels();
    });

    DebugOptions::show_history_window([&] {
        ImGui::PushFont(Cool::Font::monospace());
        project().history.imgui_show([](ReversibleCommand const& command) {
            return command_to_string(command);
        });
        ImGui::PopFont();
    });
    if (DebugOptions::show_nodes_and_links_registries())
    {
        project().modules_graph->debug_show_nodes_and_links_registries_windows(ui());
    }

    DebugOptions::test_shaders_compilation__window([&]() {
        if (ImGui::Button("Compile everything"))
        {
            Cool::message_console().clear();
            compile_all_is0_nodes();
        }
        ImGui::Separator();
        if (ImGui::Button("Compile all is0 Nodes"))
        {
            Cool::message_console().clear();
            compile_all_is0_nodes();
        }
    });
    Cool::debug_options_windows(&_tips_manager, _main_window);
}

void App::save_as()
{
    auto const path = _project_manager.file_dialog_to_save_project();
    if (!path.has_value())
        return;

    command_execution_context().execute(Command_SaveProjectAs{
        *path,
        {
            .register_project_in_the_launcher = true,
            .allow_overwrite_existing_file    = true,
        },
    });
}

void App::file_menu()
{
    if (ImGui::BeginMenu(Cool::icon_fmt("File", ICOMOON_FILE_TEXT2, true).c_str()))
    {
        auto const ctx = command_execution_context();
        if (ImGui::MenuItem("Save", ctrl_or_cmd "+S"))
            ctx.execute(Command_SaveProject{.is_autosave = false, .must_absolutely_succeed = false});
        if (ImGui::MenuItem("Save As", ctrl_or_cmd "+Shift+S"))
        {
            save_as();
        }
        if (DebugOptions::allow_user_to_open_any_file())
        {
            if (ImGui::MenuItem("Open", ctrl_or_cmd "+O"))
            {
                auto const path = _project_manager.file_dialog_to_open_project();
                if (path)
                    ctx.execute(Command_OpenProjectOnNextFrame{*path});
            }
        }
        ImGui::EndMenu();
    }
}

void App::performance_menu()
{
    if (ImGui::BeginMenu(Cool::icon_fmt("Performance", ICOMOON_POWER, true).c_str()))
    {
        project().view_constraint.imgui_nb_pixels();
        ImGui::EndMenu();
    }
}

// void App::windows_menu()
// {
//     if (ImGui::BeginMenu("Windows"))
//     {
//         for (auto& view : _views)
//         {
//             view.view.imgui_open_close_checkbox();
//         }
//         ImGui::EndMenu();
//     }
// }

void App::export_menu()
{
    if (ImGui::BeginMenu(Cool::icon_fmt("Export", ICOMOON_UPLOAD2, true).c_str()))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{0.f, 0.5f});
        project().exporter.imgui_menu_items(
            {
                .open_image_exporter = [&]() { command_executor().execute(Command_OpenImageExporter{}); },
                .open_video_exporter = [&]() { command_executor().execute(Command_OpenVideoExporter{}); },
            },
            Cool::icon_fmt("Share online", ICOMOON_EARTH, true)
        );
        _gallery_publisher.imgui_open_sharing_form();
        ImGui::PopStyleVar();
        ImGui::EndMenu();
    }
}

void App::settings_menu()
{
    if (ImGui::BeginMenu(Cool::icon_fmt("Settings", ICOMOON_COG, true).c_str()))
    {
        Cool::ImGuiExtras::separator_text("History");
        project().history.imgui_max_size(&Cool::ImGuiExtras::help_marker);
        project().history.imgui_max_saved_size(&Cool::ImGuiExtras::help_marker);

        Cool::ImGuiExtras::separator_text("Color Theme");
        Cool::color_themes()->imgui_theme_picker();

        Cool::user_settings().imgui();

        user_settings().imgui();

        ImGui::EndMenu();
    }
}

void App::commands_menu()
{
    if (ImGui::BeginMenu(Cool::icon_fmt("Commands", ICOMOON_ROCKET, true).c_str()))
    {
        if (ImGui::Selectable(ICOMOON_BUBBLE " Show all the tips"))
            _tips_manager.open_all_tips_window();
        if (ImGui::Selectable(ICOMOON_VIDEO_CAMERA " Open Webcams config"))
            Cool::WebcamsConfigs::instance().open_imgui_window();
        if (ImGui::Selectable(ICOMOON_EQUALIZER2 " Open MIDI config"))
            Cool::midi_manager().open_config_window();
        if (ImGui::Selectable(ICOMOON_CONNECTION " Open OSC config"))
            Cool::osc_manager().open_config_window();
        if (ImGui::Selectable(ICOMOON_CONNECTION " Open Server config"))
            Cool::server_manager().open_config_window();
        if (ImGui::Selectable(ICOMOON_MUSIC " Open Audio config"))
            project().audio.open_imgui_window();
        if (ImGui::Selectable(ICOMOON_IMAGE " Open output window"))
        {
            _output_view.toggle_open_close();
            if (_output_view.is_open())
                project().shared_aspect_ratio.fill_the_view = true;
        }
        if (ImGui::Selectable(ICOMOON_IMAGE " Spout/Syphon OUT config"))
            spout_out_manager().open_imgui_window();
        if (ImGui::Selectable(ICOMOON_FOLDER_OPEN " Open user-data folder"))
            Cool::open_folder_in_explorer(Cool::Path::user_data());
        ImGui::EndMenu();
    }
}

void App::debug_menu()
{
    if (ImGui::BeginMenu("Debug"))
    {
        DebugOptionsManager::imgui_ui_for_all_options();
        ImGui::EndMenu();
    }
}

void App::about_menu()
{
    if (ImGui::BeginMenu("About"))
    {
        if (ImGui::Button("Website"))
            Cool::open_link("https://coollab-art.com/");
        ImGui::SetItemTooltip("%s", "https://coollab-art.com/");
        if (ImGui::Button("License"))
            _license_window_is_open = true;

        ImGui::EndMenu();
    }
}

void App::imgui_menus()
{
    file_menu();
    export_menu();
    // windows_menu();/// This menu might make sense if we have several views one day, but for now it just creates a menu for no reason
    settings_menu();
    performance_menu();
    commands_menu();

    _project_manager.imgui_project_name_in_the_middle_of_the_menu_bar(make_window_title_setter());
    ImGui::Dummy({
        // HACK while waiting for ImGui to support right-to-left layout. See issue https://github.com/ocornut/imgui/issues/5875
        ImGui::GetContentRegionAvail().x
            - ImGui::CalcTextSize("AboutDebug").x
            - 4.f * Cool::ImGuiExtras::GetStyle().menu_bar_spacing.x
            - std::max(Cool::ImGuiExtras::GetStyle().menu_bar_spacing.x - ImGui::GetStyle().WindowPadding.x, 0.f),
        0.f,
    });
    about_menu();
    debug_menu();
}

void App::reset_cameras()
{
    auto executor = command_executor();
    project().camera_2D_manager.reset_camera(executor);
    project().camera_3D_manager.reset_camera(executor);
}

void App::check_inputs()
{
    if (ImGui::GetIO().WantTextInput)
        return;

    check_inputs__history();
    check_inputs__project();
    check_inputs__timeline();
    if (ImGui::IsKeyChordPressed(ImGuiKey_Escape))
    {
        _wants_view_in_fullscreen = false;
    }
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_E))
    {
        auto const exported_image_path = project().exporter.export_image_with_current_settings_using_a_task(project().clock.time(), project().clock.delta_time(), polaroid(), image_export_path_checks());
        on_image_export_start(exported_image_path);
    }
}

void App::check_inputs__history()
{
    auto exec = reversible_command_executor_without_history();

    // Undo
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Z))
    {
        project().history.move_backward(exec);
    }

    // Redo
    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Y)
        || ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z))
    {
        project().history.move_forward(exec);
    }
}

void App::check_inputs__project()
{
    auto const ctx = command_execution_context();

    if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_S))
    {
        save_as();
    }
    else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_S))
    {
        ctx.execute(Command_SaveProject{.is_autosave = false, .must_absolutely_succeed = false});
    }
    else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_O))
    {
        if (DebugOptions::allow_user_to_open_any_file())
        {
            auto const path = _project_manager.file_dialog_to_open_project();
            if (path)
                ctx.execute(Command_OpenProjectOnNextFrame{*path});
        }
    }
    else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_KeypadAdd)
             || ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Equal))
    {
        Cool::user_settings().change_ui_zoom(+1.f);
    }
    else if (ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_KeypadSubtract)
             || ImGui::IsKeyChordPressed(ImGuiMod_Ctrl | ImGuiKey_Minus))
    {
        Cool::user_settings().change_ui_zoom(-1.f);
    }
}

void App::check_inputs__timeline()
{
    if (ImGui::IsKeyChordPressed(ImGuiKey_Space))
    {
        project().clock.toggle_play_pause();
    }
}

void App::open_image_exporter()
{
    project().exporter.image_export_window().open();
}

void App::open_video_exporter()
{
    project().exporter.video_export_window().open();
}

void App::open_meshing_window_for_node(Cool::NodeId const& node_id)
{
    project().meshing_gui.open_window(node_id);
}

} // namespace Lab
