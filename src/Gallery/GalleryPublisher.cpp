#include "GalleryPublisher.hpp"
#include <imgui.h>
#include "Cool/DebugOptions/DebugOptions.h"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/icon_fmt.h"
#include "Cool/ImGui/markdown.h"
#include "Cool/Image/compute_image_size.h"
#include "Cool/Task/TaskManager.hpp"
#include "Task_PublishImageToGallery.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace Lab {

GalleryPublisher::GalleryPublisher()
    : _window{Cool::icon_fmt("Share online", ICOMOON_EARTH), Cool::ImGuiWindowConfig{.is_modal = true}}
    , _serializer{
          "gallery_publisher.json",
          true /*autosave_when_destroyed*/,
          [&](nlohmann::json const& json) {
              Cool::json_get(json, "Artwork info", _artwork_info);
              Cool::json_get(json, "Author info", _author_info);
              Cool::json_get(json, "Legal info", _legal_info);
          },
          [&](nlohmann::json& json) {
              Cool::json_set(json, "Artwork info", _artwork_info);
              Cool::json_set(json, "Author info", _author_info);
              Cool::json_set(json, "Legal info", _legal_info);
          },
          false /*use_shared_user_data*/
      }
{
}

static constexpr bool has_openssl =
#if CPPHTTPLIB_OPENSSL_SUPPORT
    true;
#else
    false;
#endif

void GalleryPublisher::imgui_open_sharing_form()
{
    Cool::ImGuiExtras::disabled_if(
        !has_openssl,
        "DEV ONLY: We didn't find the OpenSSL library on your machine while compiling Coollab so this feature was disabled.\nLook at how to install OpenSSL on your computer if you want this feature.",
        [&]() {
            if (ImGui::Button(Cool::icon_fmt("Share online", ICOMOON_EARTH, true).c_str()))
                _window.open();
        }
    );
}

void GalleryPublisher::imgui_window(std::function<img::Image(img::Size)> const& render_image)
{
    _window.show([&](bool /*is_opening*/) {
        bool b{false};
        Cool::ImGuiExtras::markdown("Your image will be visible on [Coollab's gallery](https://coollab-art.com/Gallery).");
        Cool::ImGuiExtras::markdown("If you want to edit or remove it, send an email at [coollab.lib@gmail.com](mailto:coollab.lib@gmail.com) from the email address that you will provide below.");
        Cool::ImGuiExtras::separator_text("Artwork");
        b |= _artwork_info.imgui();
        _shared_aspect_ratio->fill_the_view = false; // Fixed aspect ratio as soon as we open the export window. This avoids the case where we open the export window, then resize the main window which makes the View change aspect ratio. In that case we don't want the current aspect ratio and size that we see in the export window to change. And to preserve the consistency between the aspect ratio in the exporter window and in the View, we have to keep the aspect ratio fixed.
        _shared_aspect_ratio->aspect_ratio.imgui(0.f, "Aspect Ratio");
        {
            auto const size = export_size();
            auto       sz   = glm::ivec2{
                static_cast<int>(size.width()),
                static_cast<int>(size.height()),
            };
            ImGui::BeginDisabled();
            ImGui::DragInt2("Size", glm::value_ptr(sz));
            ImGui::EndDisabled();
            Cool::ImGuiExtras::help_marker("You cannot choose the size of your image because we don't want to store images that are too big.\nYou can only control the Aspect Ratio above.");
        }
        Cool::ImGuiExtras::separator_text("Author");
        b |= _author_info.imgui();
        Cool::ImGuiExtras::separator_text("Legal");
        b |= _legal_info.imgui();
        Cool::ImGuiExtras::separator_text("");
        auto const missing_info_message = std::optional<std::string>{_legal_info.missing_information_message()};
        Cool::ImGuiExtras::disabled_if(missing_info_message.has_value(), missing_info_message.value_or("").c_str(), [&]() {
            if (ImGui::Button(Cool::icon_fmt("Send", ICOMOON_EARTH).c_str()))
            {
                Cool::task_manager().submit(std::make_shared<Task_PublishImageToGallery>(_artwork_info, _author_info, _legal_info, render_image(export_size())));
                _window.close();
                _artwork_info = {};            // Clear info that is related to one specific artwork
                _legal_info.reset_agreement(); // Force them to agree explicitly each time, to make sure they don't agree by accident
                if (Cool::DebugOptions::public_exhibition_mode())
                {
                    _author_info = {}; // Clear info that is related to the author,
                    _legal_info  = {}; // because the next post will be made by another visitor.
                }
            }
        });

        if (b)
            _serializer.save();
    });
}

auto GalleryPublisher::export_size() const -> img::Size
{
    return Cool::compute_image_size(_shared_aspect_ratio->aspect_ratio.get(), 2'250'000.f);
}

} // namespace Lab