#include "ProjectManager.hpp"
#include <ImGuiNotify/ImGuiNotify.hpp>
#include <filesystem>
#include <reg/src/generate_uuid.hpp>
#include "COOLLAB_FILE_EXTENSION.hpp"
#include "CommandCore/execute_command.hpp"
#include "Command_OpenProjectOnNextFrame.hpp"
#include "Cool/CommandLineArgs/CommandLineArgs.h"
#include "Cool/File/File.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/ImGuiExtrasStyle.h"
#include "Cool/Log/boxer_show.hpp"
#include "Cool/Path/Path.h"
#include "Cool/UserSettings/UserSettings.h"
#include "Debug/DebugOptions.h"
#include "ProjectManagerImpl.hpp"
#include "SaveAsOptions.hpp"
#include "UserSettings/UserSettings.hpp"

namespace Lab {

static auto get_argument_after(size_t& i) -> std::string const*
{
    assert(i < Cool::command_line_args().get().size());

    if (i + 1 == Cool::command_line_args().get().size()
        || Cool::command_line_args().get()[i + 1].starts_with("--"))
    {
        ImGuiNotify::send({
            .type    = ImGuiNotify::Type::Warning,
            .title   = "Project Command-Line Arguments",
            .content = fmt::format("Missing argument after \"{}\"", Cool::command_line_args().get()[i]),
        });
        return nullptr;
    }

    i++;
    return &Cool::command_line_args().get()[i];
}

void ProjectManager::process_command_line_args(OnProjectLoaded const& on_project_loaded, OnProjectUnloaded const& on_project_unloaded, SetWindowTitle const& set_window_title)
{
    for (size_t i = 0; i < Cool::command_line_args().get().size(); ++i)
    {
        auto const& arg = Cool::command_line_args().get()[i];

        if (arg == "--create_new_project")
        {
            create_new_project(on_project_loaded, set_window_title);
        }
        else if (arg == "--create_new_project_in_folder")
        {
            auto const* const arg2 = get_argument_after(i); // NB: this might increment i
            if (arg2)
                create_new_project_in_folder(*arg2, on_project_loaded, set_window_title);
        }
        else if (arg == "--create_new_project_in_file")
        {
            auto const* const arg2 = get_argument_after(i); // NB: this might increment i
            if (arg2)
                create_new_project_in_file(*arg2, on_project_loaded, set_window_title);
        }
        else if (arg == "--open_project")
        {
            auto const* const arg2 = get_argument_after(i); // NB: this might increment i
            if (arg2)
                open_project(*arg2, on_project_loaded, on_project_unloaded, set_window_title);
        }
        else if (arg == "--projects_info_folder_for_the_launcher")
        {
            auto const* const arg2 = get_argument_after(i); // NB: this might increment i
            if (arg2)
                _impl.set_projects_info_folder_for_the_launcher(*arg2);
        }
        else
        {
            ImGuiNotify::send({
                .type    = ImGuiNotify::Type::Warning,
                .title   = "Project Command-Line Arguments",
                .content = fmt::format("Unknown argument \"{}\"", arg),
            });
        }
    }
    // In case no command line args told us what to do (which should only happen when Coollab is not launched via the launcher, i.e. only while developing Coollab)
    // then always open the same project, so that it is convenient while developing to access your test project immediately
    if (_impl.project_folder().empty())
    {
        auto const path = Cool::File::with_extension(Cool::Path::user_data() / "Projects/Untitled", COOLLAB_FILE_EXTENSION);
        if (Cool::File::exists(path))
            open_project(path, on_project_loaded, on_project_unloaded, set_window_title);
        else
            create_new_project_in_file(path, on_project_loaded, set_window_title);
    }
}

void ProjectManager::create_new_project(OnProjectLoaded const& on_project_loaded, SetWindowTitle const& set_window_title)
{
    create_new_project_in_folder(Cool::Path::user_data() / "Projects", on_project_loaded, set_window_title);
}

void ProjectManager::create_new_project_in_folder(std::filesystem::path const& folder_path, OnProjectLoaded const& on_project_loaded, SetWindowTitle const& set_window_title)
{
    create_new_project_in_file(Cool::File::with_extension(folder_path / "Untitled", COOLLAB_FILE_EXTENSION), on_project_loaded, set_window_title);
}

void ProjectManager::create_new_project_in_file(std::filesystem::path file_path, OnProjectLoaded const& on_project_loaded, SetWindowTitle const& set_window_title)
{
    if (DebugOptions::log_project_related_events())
        Cool::Log::info("Project", fmt::format("Creating new project in \"{}\"", Cool::File::weakly_canonical(file_path)));

    auto project = Project{};
    project.clock.pause();
    project.camera_3D_manager.is_editable_in_view() = false;
    _impl.set_project(std::move(project), on_project_loaded);

    file_path = Cool::File::find_available_path(file_path, Cool::PathChecks{});
    // Save immediately, so that no one will try to create another project with the same name, thinking the name is not in use
    std::ignore = save_project_impl(file_path, true /*must_absolutely_succeed*/, set_window_title);
}

void ProjectManager::open_project(std::filesystem::path const& file_path, OnProjectLoaded const& on_project_loaded, OnProjectUnloaded const& on_project_unloaded, SetWindowTitle const& set_window_title)
{
    if (DebugOptions::log_project_related_events())
        Cool::Log::info("Project", fmt::format("Opening project \"{}\"", Cool::File::weakly_canonical(file_path)));

    if (_impl.has_project())
    {
        if (!save_project_impl(false /*must_absolutely_succeed*/, set_window_title))
        {
            ImGuiNotify::send({
                .type     = ImGuiNotify::Type::Error,
                .title    = "Failed to open project",
                .content  = fmt::format("We failed to save the current project, so we didn't open the new one because we would have lost the changes to the current project.\n{}", Cool::File::weakly_canonical(file_path)),
                .duration = std::nullopt,
            });
            return;
        }
        on_project_unloaded();
    }

    auto maybe_project = _impl.load(file_path);
    if (!maybe_project.has_value())
    {
        ImGuiNotify::send({
            .type     = ImGuiNotify::Type::Error,
            .title    = "Failed to open project",
            .content  = maybe_project.error() + fmt::format("\n{}", Cool::File::weakly_canonical(file_path)),
            .duration = std::nullopt,
        });
        create_new_project_in_folder(Cool::File::without_file_name(file_path), on_project_loaded, set_window_title);
        return;
    }

    _impl.set_project(std::move(maybe_project.value()), on_project_loaded);
    _impl.set_project_path(file_path, set_window_title);
    _impl.register_last_write_time(file_path);
}

auto ProjectManager::autosave_project(bool must_absolutely_succeed, SetWindowTitle const& set_window_title) -> bool
{
    if (DebugOptions::log_project_related_events())
        Cool::Log::info("Project", fmt::format("Autosaving project \"{}\"", Cool::File::weakly_canonical(_impl.project_path())));

    return save_project_impl(must_absolutely_succeed, set_window_title);
}

auto ProjectManager::save_project(bool must_absolutely_succeed, SetWindowTitle const& set_window_title) -> bool
{
    if (DebugOptions::log_project_related_events())
        Cool::Log::info("Project", fmt::format("Saving project \"{}\"", Cool::File::weakly_canonical(_impl.project_path())));

    if (!save_project_impl(must_absolutely_succeed, set_window_title))
        return false;

    ImGuiNotify::send({
        .type     = ImGuiNotify::Type::Success,
        .title    = "Saved Project",
        .content  = Cool::user_settings().autosave_enabled
                        ? fmt::format("NB: there is an autosave every {} seconds, so you don't really need to save the project manually", Cool::user_settings().autosave_delay.as_seconds_float())
                        : "",
        .duration = 2s,
    });

    return true;
}

auto ProjectManager::save_project_impl(bool must_absolutely_succeed, SetWindowTitle const& set_window_title) -> bool
{
    return save_project_impl(_impl.project_path(), must_absolutely_succeed, set_window_title);
}

auto ProjectManager::save_project_impl(std::filesystem::path file_path, bool must_absolutely_succeed, SetWindowTitle const& set_window_title) -> bool
{
    auto const old_path = _impl.project_path();
    if (_impl.file_contains_data_that_we_did_not_write_ourselves(file_path))
    {
        // We would overwrite a file that we did not write ourselves, this is dangerous because we might overwrite data saved by the user with another application
        while (true)
        {
            Cool::boxer_show(
                fmt::format("This file has been modified externally. We cannot save there because it would overwrite the changes.\nPlease select another location to save the file.\n{}", Cool::File::weakly_canonical(file_path)).c_str(),
                "Cannot save here",
                boxer::Style::Warning,
                boxer::Buttons::OK
            );
            auto const path = file_dialog_to_save_project();
            if (!path.has_value())
            {
                if (must_absolutely_succeed)
                    continue;
                else // NOLINT(*else-after-return)
                    return false;
            }
            file_path = *path;
            break;
        }
    }

    while (true)
    {
        auto const maybe_err = project_path_error_message(file_path, {.allow_overwrite_existing_file = true});
        if (maybe_err.has_value())
        {
            Cool::boxer_show(
                fmt::format("{}\nPlease choose another name for your project\n{}", *maybe_err, Cool::File::weakly_canonical(file_path)).c_str(),
                "Invalid name",
                boxer::Style::Warning,
                boxer::Buttons::OK
            );
        }
        else if (!_impl.save(file_path))
        {
            Cool::boxer_show(
                fmt::format("Save failed.\nPlease select another location to save the file.\n{}", Cool::File::weakly_canonical(file_path)).c_str(),
                "Cannot save here",
                boxer::Style::Warning,
                boxer::Buttons::OK
            );
        }
        else
        {
            break; // Save succeeded
        }

        auto const path = file_dialog_to_save_project();
        if (!path.has_value())
        {
            if (must_absolutely_succeed)
                continue;
            else // NOLINT(*else-after-return)
                return false;
        }
        file_path = *path;
    }

    // if (file_path != _impl.project_path()) // Do it all the time, so that if we open a file that was not registered in the launcher, it now will be
    {
        _impl.set_project_path(file_path, set_window_title);
        _impl.set_project_path_in_info_folder_for_the_launcher(file_path);
    }
    _impl.register_last_write_time(file_path);
    if (old_path != file_path && !_impl.file_contains_data_that_we_did_not_write_ourselves(old_path))
        _impl.remove_info_folder_for_the_launcher(old_path);

    return true;
}

auto ProjectManager::save_project_as(std::filesystem::path file_path, SaveThumbnail const& save_thumbnail, SaveAsOptions options) -> bool
{
    if (DebugOptions::log_project_related_events())
        Cool::Log::info("Project", fmt::format("Saving project as \"{}\"", Cool::File::weakly_canonical(file_path)));

    while (true)
    {
        auto const maybe_err = project_path_error_message(file_path, {.allow_overwrite_existing_file = options.allow_overwrite_existing_file});
        if (maybe_err.has_value())
        {
            Cool::boxer_show(
                fmt::format("{}\nPlease choose another name for your project\n{}", *maybe_err, Cool::File::weakly_canonical(file_path)).c_str(),
                "Invalid name",
                boxer::Style::Warning,
                boxer::Buttons::OK
            );
        }
        else if (!_impl.save(file_path))
        {
            Cool::boxer_show(
                fmt::format("Save failed.\nPlease select another location to save the file.\n{}", Cool::File::weakly_canonical(file_path)).c_str(),
                "Cannot save here",
                boxer::Style::Warning,
                boxer::Buttons::OK
            );
        }
        else
        {
            break; // Save succeeded
        }

        auto const path = file_dialog_to_save_project();
        if (!path.has_value())
            return false;
        file_path = *path;
    }

    if (options.register_project_in_the_launcher)
    {
        _impl.set_project_path_in_info_folder_for_the_launcher(file_path);
        auto const info_folder = _impl.info_folder_for_the_launcher(file_path);
        if (info_folder)
            save_thumbnail(*info_folder);
    }

    bool const wants_to_switch_to_new_project = !user_settings().save_as_behaves_as_save_backup
                                                && options.register_project_in_the_launcher; // If we don't register this project in the launcher, then we shouldn't make it the current project either)

    if (wants_to_switch_to_new_project)
        open_project_on_next_frame(file_path);

    if (options.register_project_in_the_launcher)
    {
        ImGuiNotify::send({
            .type                 = ImGuiNotify::Type::Success,
            .title                = fmt::format("Saved as"),
            .content              = Cool::File::weakly_canonical(file_path).string(),
            .custom_imgui_content = [wants_to_switch_to_new_project, file_path, old_file_path = _impl.project_path()]() {
                if (wants_to_switch_to_new_project)
                {
                    if (ImGui::Button(fmt::format("Switch back to \"{}\"", Cool::File::file_name_without_extension(old_file_path)).c_str()))
                        execute_command(Command_OpenProjectOnNextFrame{old_file_path});
                    Cool::ImGuiExtras::help_marker("If you always want to switch back you can make this the default behaviour by enabling \"Save As behaves as a Save Backup\" in the  " ICOMOON_COG "Settings menu");
                }
                else
                {
                    if (ImGui::Button(fmt::format("Switch to \"{}\"", Cool::File::file_name_without_extension(file_path)).c_str()))
                        execute_command(Command_OpenProjectOnNextFrame{file_path});
                    Cool::ImGuiExtras::help_marker("If you always want to switch you can make this the default behaviour by disabling \"Save As behaves as a Save Backup\" in the  " ICOMOON_COG "Settings menu");
                }
            },
            .duration = 5s,
        });
    }

    return true;
}

static auto file_path_to_package_project_into(std::filesystem::path folder_path) -> std::filesystem::path
{
    folder_path.replace_extension(COOLLAB_FILE_EXTENSION);
    return folder_path;
}

auto is_valid_path_to_package_project_into(std::filesystem::path const& folder_path) -> bool
{
    // TODO(Project) once we really save in a folder, it's okay if the folder exists, as long as it is empty
    auto const file_path = file_path_to_package_project_into(folder_path);
    return !Cool::File::exists(file_path)
        // && is_valid_project_path(file_path) // NB: we need to be able to find another valid path with the same name and an extra number at the end at the end (e.g. "MyProj (5)"). Otherwise we will loop infinitely to find an available project name. This is why we don't check that here, but it will be checked later when we save the project as.
        ;
}

auto ProjectManager::package_project_into(std::filesystem::path const& folder_path, SaveThumbnail const& save_thumbnail, SaveAsOptions options) -> bool
{
    if (DebugOptions::log_project_related_events())
        Cool::Log::info("Project", fmt::format("Packaging project into \"{}\"", Cool::File::weakly_canonical(folder_path)));

    if (!is_valid_path_to_package_project_into(folder_path))
    {
        ImGuiNotify::send({
            .type     = ImGuiNotify::Type::Error,
            .title    = "Failed to package project",
            .content  = fmt::format("This file already exists. Please choose another one.\n{}", Cool::File::weakly_canonical(folder_path)), // TODO(Project) replace text with "this folder" once we package into a folder
            .duration = std::nullopt,
        });
        return false;
    }

    return save_project_as(file_path_to_package_project_into(folder_path), save_thumbnail, options); // TODO(Project) Implement the packaging-specific stuff like copying images and nodes.
}

auto ProjectManager::rename_project(std::string new_name, SetWindowTitle const& set_window_title) -> bool
{
    if (DebugOptions::log_project_related_events())
        Cool::Log::info("Project", fmt::format("Renaming project as \"{}\"", new_name));

    auto const old_path = _impl.project_path();
    if (!save_project_impl(_impl.project_path(new_name), false, set_window_title))
        return false;

    if (!_impl.file_contains_data_that_we_did_not_write_ourselves(old_path))
    {
        Cool::File::remove_file(old_path);
        _impl.remove_info_folder_for_the_launcher(old_path);
    }

    return true;
}

auto ProjectManager::project_name_error_message(std::string const& name, NameValidityChecks checks) const -> std::optional<std::string>
{
    if (!checks.allow_overwrite_existing_file)
    {
        if (Cool::File::exists(_impl.project_path(name)) && name != _impl.project_name())
            return "Name already used by another project";
    }

    if (name.empty())
        return "Name cannot be empty";

    for (char const invalid_char : {'.', '<', '>', ':', '\"', '/', '\\', '|', '?', '*', '\0'})
    {
        if (name.find(invalid_char) != std::string::npos)
            return fmt::format("Name cannot contain a {}", invalid_char);
    }

    if (name.ends_with(' '))
        return "Name cannot end with a space";

    if (name.starts_with("--"))
        return "Name cannot start with --"; // Otherwise, when passing this file name as a command-line argument, we would think it's an argument and not a file name

    {
        auto upper_case_name = name;
        std::transform(upper_case_name.begin(), upper_case_name.end(), upper_case_name.begin(), [](char c) {
            return static_cast<char>(std::toupper(static_cast<unsigned char>(c))); // We need those static_casts to avoid undefined behaviour, cf. https://en.cppreference.com/w/cpp/string/byte/toupper
        });
        for (const char* invalid_name : {
                 "CON", "PRN", "AUX", "NUL",
                 "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9",
                 "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"
             })
        {
            if (upper_case_name == invalid_name)
                return fmt::format("{} is a reserved name", name);
        }
    }

    return std::nullopt;
}

auto ProjectManager::project_path_error_message(std::filesystem::path const& file_path, NameValidityChecks checks) const -> std::optional<std::string>
{
    return project_name_error_message(Cool::File::file_name_without_extension(file_path).string(), checks);
}

auto ProjectManager::is_valid_project_name(std::string const& name, NameValidityChecks checks) const -> bool
{
    return !project_name_error_message(name, checks).has_value();
}

auto ProjectManager::is_valid_project_path(std::filesystem::path const& file_path, NameValidityChecks checks) const -> bool
{
    return is_valid_project_name(Cool::File::file_name_without_extension(file_path).string(), checks);
}

void ProjectManager::imgui_project_name_in_the_middle_of_the_menu_bar(SetWindowTitle const& set_window_title)
{
    auto project_name = _next_project_name.value_or(_impl.project_name());
    // TODO(Launcher) make this an ImGuiExtras widget
    auto const width = ImGui::CalcTextSize(project_name.c_str()).x
                       + 2.f * Cool::ImGuiExtras::GetStyle().tab_bar_padding.x;
    ImGui::SetNextItemWidth(width);
    ImGui::SetCursorPosX(ImGui::GetWindowSize().x * 0.5f - width * 0.5f);

    if (ImGui::InputText("##project_name", &project_name))
        _next_project_name = project_name;

    auto const name_validity_checks = NameValidityChecks{.allow_overwrite_existing_file = false};
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        assert(_next_project_name.has_value());
        if (is_valid_project_name(*_next_project_name, name_validity_checks) && *_next_project_name != _impl.project_name())
            rename_project(*_next_project_name, set_window_title);
        _next_project_name.reset();
    }
    auto const maybe_err = project_name_error_message(project_name, name_validity_checks);
    if (maybe_err.has_value())
        Cool::ImGuiExtras::warning_text(maybe_err->c_str());
}

auto ProjectManager::file_dialog_to_save_project() -> std::optional<std::filesystem::path>
{
    return Cool::File::file_saving_dialog(Cool::File::file_dialog_args{
        .file_filters   = {{"Coollab project", COOLLAB_FILE_EXTENSION}},
        .initial_folder = _impl.project_folder(),
    });
}

auto ProjectManager::file_dialog_to_open_project() -> std::optional<std::filesystem::path>
{
    return Cool::File::file_opening_dialog(Cool::File::file_dialog_args{
        .file_filters   = {{"Coollab project", COOLLAB_FILE_EXTENSION}},
        .initial_folder = _impl.project_folder(),
    });
}

void ProjectManager::open_project_on_next_frame(std::filesystem::path const& file_path)
{
    _project_to_open_on_next_frame = file_path;
}

void ProjectManager::open_requested_project_if_any(OnProjectLoaded const& on_project_loaded, OnProjectUnloaded const& on_project_unloaded, SetWindowTitle const& set_window_title)
{
    if (_project_to_open_on_next_frame.has_value())
    {
        open_project(*_project_to_open_on_next_frame, on_project_loaded, on_project_unloaded, set_window_title);
        _project_to_open_on_next_frame.reset();
    }
}

} // namespace Lab