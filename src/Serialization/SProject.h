#pragma once
#include <filesystem>
#include "Cool/Log/ErrorMessage.hpp"
#include "Project.hpp"

namespace Lab {

auto do_save(Project const&, std::filesystem::path const&) -> bool;
auto do_load(Project&, std::filesystem::path const&) -> tl::expected<void, Cool::ErrorMessage>;

} // namespace Lab