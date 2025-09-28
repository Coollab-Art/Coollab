#pragma once
#include "Cool/View/TextureView.hpp"

namespace Lab {

/// HACK to allow the project to access the output view and serialize it in the project file
inline auto output_view_ptr() -> Cool::TextureView*&
{
    static Cool::TextureView* instance{nullptr};
    return instance;
}

} // namespace Lab