#pragma once
#include "Project.hpp"
#include "ProjectManagerImpl.hpp"
#include "SaveAsOptions.hpp"

namespace Lab {

auto is_valid_path_to_package_project_into(std::filesystem::path const& folder_path) -> bool;

class ProjectManager {
public:
    auto project() -> Project& { return _impl.project(); }
    auto project() const -> Project const& { return _impl.project(); }
    auto project_path() const -> std::filesystem::path { return _impl.project_path(); }
    auto info_folder_for_the_launcher() const -> std::optional<std::filesystem::path> { return _impl.info_folder_for_the_launcher(); }
    auto has_registered_project_to_the_launcher() const -> bool { return _impl.has_registered_project_to_the_launcher(); }

    void process_command_line_args(OnProjectLoaded const&, OnProjectUnloaded const&, SetWindowTitle const&);
    void create_new_project(OnProjectLoaded const&, SetWindowTitle const&);
    void create_new_project_in_folder(std::filesystem::path const& folder_path, OnProjectLoaded const&, SetWindowTitle const&);
    void create_new_project_in_file(std::filesystem::path file_path, OnProjectLoaded const&, SetWindowTitle const&);

    // We only open projects at the beginning of a frame, because
    void open_project_on_next_frame(std::filesystem::path const& file_path);
    void open_requested_project_if_any(OnProjectLoaded const&, OnProjectUnloaded const&, SetWindowTitle const&);

    auto autosave_project(bool must_absolutely_succeed, SetWindowTitle const&) -> bool;
    auto save_project(bool must_absolutely_succeed, SetWindowTitle const&) -> bool;
    auto save_project_as(std::filesystem::path file_path, SaveThumbnail const&, SaveAsOptions = {}) -> bool;
    auto package_project_into(std::filesystem::path const& folder_path, SaveThumbnail const&, SaveAsOptions = {}) -> bool;
    auto rename_project(std::string new_name, SetWindowTitle const&) -> bool;

    void imgui_project_name_in_the_middle_of_the_menu_bar(SetWindowTitle const&);

    auto file_dialog_to_save_project() -> std::optional<std::filesystem::path>;
    auto file_dialog_to_open_project() -> std::optional<std::filesystem::path>;

private:
    /// Returns true iff we saved successfully
    [[nodiscard]] auto save_project_impl(bool must_absolutely_succeed, SetWindowTitle const&) -> bool;
    [[nodiscard]] auto save_project_impl(std::filesystem::path file_path, bool must_absolutely_succeed, SetWindowTitle const&) -> bool;

    void open_project(std::filesystem::path const& file_path, OnProjectLoaded const&, OnProjectUnloaded const&, SetWindowTitle const&);

    struct NameValidityChecks {
        bool allow_overwrite_existing_file{false};
    };
    auto project_name_error_message(std::string const& name, NameValidityChecks) const -> std::optional<std::string>;
    auto project_path_error_message(std::filesystem::path const& file_path, NameValidityChecks) const -> std::optional<std::string>;
    auto is_valid_project_name(std::string const& name, NameValidityChecks) const -> bool;
    auto is_valid_project_path(std::filesystem::path const& file_path, NameValidityChecks) const -> bool;

private:
    internal::ProjectManagerImpl         _impl{};
    std::optional<std::filesystem::path> _project_to_open_on_next_frame{};

    std::optional<std::string> _next_project_name{};
};

} // namespace Lab