#include "Project.hpp"

namespace Lab {

Project::Project()
    : camera_3D_manager{"3D Camera", modules_graph->rerender_all_flag()} // TODO(Modules) Actually, some modules don't depend on the camera, so we shouldn't rerender everything
    , camera_2D_manager{"2D Camera", modules_graph->rerender_all_flag()} // TODO(Modules) Actually, some modules don't depend on the camera, so we shouldn't rerender everything
{}

} // namespace Lab