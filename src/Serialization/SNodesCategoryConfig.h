#pragma once
#include <filesystem>
#include "Cool/Log/ErrorMessage.hpp"
#include "Nodes/NodesCategoryConfig.h"

namespace Lab {

auto do_save(NodesCategoryConfig const&, std::filesystem::path const&) -> bool;
auto do_load(NodesCategoryConfig&, std::filesystem::path const&) -> tl::expected<void, Cool::ErrorMessage>;

} // namespace Lab