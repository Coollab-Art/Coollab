#pragma once

namespace Lab {

using SetWindowTitle    = std::function<void(std::string_view)>;
using OnProjectLoaded   = std::function<void()>;
using OnProjectUnloaded = std::function<void()>;
using SaveThumbnail     = std::function<void(std::filesystem::path const& folder_path)>;

} // namespace Lab