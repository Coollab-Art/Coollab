#pragma once
#include <reg/src/AnyId.hpp>
#include "Interfaces.hpp"
#include "Project.hpp"

namespace Lab::internal {

/// Contains the low-level operations, they don't do any error checking, don't retry if a file already exists, etc.
class ProjectManagerImpl {
public:
    auto project() -> Project& { return *_project; }
    auto project() const -> Project const& { return *_project; }
    auto has_project() const -> bool { return _project.has_value(); }
    auto project_path() const -> std::filesystem::path;
    auto project_folder() const -> std::filesystem::path const& { return _folder_path; }
    auto project_name() const -> std::string const& { return _file_name; }
    auto info_folder_for_the_launcher() const -> std::optional<std::filesystem::path>;
    auto info_folder_for_the_launcher(std::filesystem::path const& project_file_path) const -> std::optional<std::filesystem::path>;
    auto has_registered_project_to_the_launcher() const -> bool;

    auto               project_path(std::string_view file_name) const -> std::filesystem::path;
    void               set_project_path(std::filesystem::path const& file_path, SetWindowTitle const& set_window_title);
    void               set_project_path_in_info_folder_for_the_launcher(std::filesystem::path const& file_path) const;
    void               set_project_name(std::string_view file_name, SetWindowTitle const& set_window_title);
    void               set_project(Project project, OnProjectLoaded const& on_project_loaded);
    void               set_projects_info_folder_for_the_launcher(std::filesystem::path folder_path) { _path_to_launcher_info_folder = std::move(folder_path); }
    [[nodiscard]] auto save(std::filesystem::path const& file_path) -> bool;
    [[nodiscard]] auto load(std::filesystem::path const& file_path) -> tl::expected<Project, std::string>;
    void               register_last_write_time(std::filesystem::path const& file_path);
    auto               file_contains_data_that_we_did_not_write_ourselves(std::filesystem::path const& file_path) const -> bool;
    void               remove_info_folder_for_the_launcher(std::filesystem::path const& project_file_path) const;

private:
    std::optional<Project> _project{};
    std::filesystem::path  _folder_path{};
    std::string            _file_name{};

    std::optional<std::filesystem::path> _path_to_launcher_info_folder{};

    std::map<std::filesystem::path, std::filesystem::file_time_type> _last_write_times{};
};

} // namespace Lab::internal