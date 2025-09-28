///
/// We put all our serialization here in a separate .cpp file
/// because including "ser20/archives/json.hpp" slows down
/// compilation by A LOT. So we want to avoid recompiling
/// this file as much as possible.
///
#include "Cool/Dump/app_version.hpp"
#include "Cool/Serialization/Serialization.h"
#include "Module_Compositing/Module_Compositing.h"
#include "Module_Default/Module_Default.hpp"
#include "Module_FeedbackLoop/Module_FeedbackLoop.hpp"
#include "Module_Particles/Module_Particles.h"
#include "SNodesCategoryConfig.h"
#include "SNodesClipboard.h"
#include "SProject.h"
#include "ser20/types/polymorphic.hpp"
//
#include "ser20/archives/json.hpp"

namespace Lab {

auto do_save(Project const& project, std::filesystem::path const& path) -> bool
{
    return Cool::Serialization::save<Project, ser20::JSONOutputArchive>(project, path, "Project", Cool::app_version());
}
auto do_load(Project& project, std::filesystem::path const& path) -> tl::expected<void, Cool::ErrorMessage>
{
    auto coollab_version = ""s; // Ignore it, we don't need it when loading the project, only the launcher needs it
    return Cool::Serialization::load<Project, ser20::JSONInputArchive>(project, path, &coollab_version);
}

auto do_save(NodesCategoryConfig const& config, std::filesystem::path const& path) -> bool
{
    return Cool::Serialization::save<NodesCategoryConfig, ser20::JSONOutputArchive>(config, path);
}
auto do_load(NodesCategoryConfig& config, std::filesystem::path const& path) -> tl::expected<void, Cool::ErrorMessage>
{
    return Cool::Serialization::load<NodesCategoryConfig, ser20::JSONInputArchive>(config, path);
}

auto string_from_nodes_clipboard(NodesClipboard const& clipboard) -> std::string
{
    auto ss = std::stringstream{};
    {
        auto archive = ser20::JSONOutputArchive{ss};
        archive(ser20::make_nvp("Coollab copied nodes, you can paste this in Coollab to paste the nodes.", clipboard));
    } // archive actual work happens during its destruction
    return ss.str();
}
auto string_to_nodes_clipboard(std::string const& string) -> NodesClipboard
{
    auto clipboard = NodesClipboard{};
    {
        auto ss      = std::stringstream{string};
        auto archive = ser20::JSONInputArchive{ss};
        archive(clipboard);
    } // archive actual work happens during its destruction
    return clipboard;
}

} // namespace Lab

SER20_REGISTER_TYPE(Lab::Module_Compositing);  // NOLINT
SER20_REGISTER_TYPE(Lab::Module_Particles);    // NOLINT
SER20_REGISTER_TYPE(Lab::Module_FeedbackLoop); // NOLINT
SER20_REGISTER_TYPE(Lab::Module_Default);      // NOLINT