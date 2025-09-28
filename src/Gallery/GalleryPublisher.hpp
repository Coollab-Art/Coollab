#pragma once
#include "ArtworkInfo.h"
#include "AuthorInfo.h"
#include "Cool/ImGui/ImGuiWindow.h"
#include "Cool/Image/SharedAspectRatio.hpp"
#include "Cool/Serialization/JsonAutoSerializer.hpp"
#include "LegalInfo.h"
#include "img/img.hpp"

namespace Lab {

class GalleryPublisher {
public:
    GalleryPublisher();

    void imgui_open_sharing_form();
    /// `render_image` is a function that renders an image of the requested size
    void imgui_window(std::function<img::Image(img::Size)> const& render_image);

    void set_shared_aspect_ratio(Cool::SharedAspectRatio& shared_aspect_ratio) { _shared_aspect_ratio = &shared_aspect_ratio; }

private:
    [[nodiscard]] auto export_size() const -> img::Size;

private:
    Cool::ImGuiWindow        _window;
    ArtworkInfo              _artwork_info{};
    AuthorInfo               _author_info{};
    LegalInfo                _legal_info{};
    Cool::SharedAspectRatio* _shared_aspect_ratio{};

    Cool::JsonAutoSerializer _serializer;
};

} // namespace Lab