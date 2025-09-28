#pragma once
#include <CommandCore/CommandExecutor_TopLevel.h>
#include "ConcreteCommand.h"
#include "Dependencies/History.h"
#include "ProjectManager/Interfaces.hpp"

namespace Lab {

class Camera3DManager;
class App;
class ProjectManager;
struct Project;
class ModulesGraph;
class CommandExecutor;
class Node;

// This is a class rather than a struct because we want to use methods to access the different members
// this will make it easier to track down who is using what
// and since this class is a big thing that makes everything accessible to everyone, it is especially important to be able to track the use that is made of each member.
class CommandExecutionContext_Ref {
public:
    [[nodiscard]] auto app() const -> App& { return _data.app; }
    [[nodiscard]] auto history() const -> History const&;
    [[nodiscard]] auto camera_manager() const -> Camera3DManager&;
    [[nodiscard]] auto project() const -> Project&;
    [[nodiscard]] auto project_manager() const -> ProjectManager& { return _data.project_manager; }
    [[nodiscard]] auto window_title_setter() const -> SetWindowTitle;
    [[nodiscard]] auto on_project_loaded() const -> OnProjectLoaded;
    [[nodiscard]] auto save_thumbnail() const -> SaveThumbnail;
    [[nodiscard]] auto command_executor() const -> CommandExecutor;
    [[nodiscard]] auto modules_graph() const -> ModulesGraph&;
    void               make_sure_node_uses_the_most_up_to_date_version_of_its_definition(Node& node) const;
    template<ConcreteCommand ConcreteCommandT>
    void execute(ConcreteCommandT const& command, bool store_in_history = true) const
    {
        _data.executor.execute(command, *this, store_in_history);
    }

    struct Data { // We wrap our members in a struct to get a constructor automatically
        std::reference_wrapper<App>            app;
        std::reference_wrapper<ProjectManager> project_manager;
        CommandExecutor_TopLevel               executor;
    };
    explicit CommandExecutionContext_Ref(Data data)
        : _data{std::move(data)}
    {}

private:
    Data _data;
};

} // namespace Lab