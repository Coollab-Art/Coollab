#include "Task_PublishImageToGallery.hpp"
#include <tl/expected.hpp>
#include "Cool/ImGui/markdown.h"
#include "Cool/String/String.h"
#include "ImGuiNotify/ImGuiNotify.hpp"
#include "cpp-httplib/httplib.h"

static auto escape(std::string str) -> std::string
{
    // First, escape the backslashes so that we don't later escape backslashes that are actually meant to escape something.
    Cool::String::replace_all_inplace(str, "\\", "\\\\");
    Cool::String::replace_all_inplace(str, "=", "\\=");
    Cool::String::replace_all_inplace(str, "|", "\\|");

    return str;
}

static auto process_link(std::string const& str) -> std::string
{
    if (str.empty() || str[0] != '@')
        return str;
    return fmt::format("https://www.instagram.com/{}", str.c_str() + 1); // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
}

#if LAB_ENABLE_TESTS
#include <doctest/doctest.h>
TEST_CASE("escape()")
{
    CHECK(escape("Hello") == "Hello");
    CHECK(escape("=") == "\\=");
    CHECK(escape("|") == "\\|");
    CHECK(escape("\\") == "\\\\");
    CHECK(escape("\\=") == "\\\\\\=");
    CHECK(escape("Hello\\=Hello") == "Hello\\\\\\=Hello");
}
TEST_CASE("process_link()")
{
    CHECK(process_link("Hello") == "Hello");
    CHECK(process_link("@julesfouchy") == "https://www.instagram.com/julesfouchy");
    CHECK(process_link("Hel@lo") == "Hel@lo");
    CHECK(process_link("@") == "https://www.instagram.com/");
    CHECK(process_link("@@") == "https://www.instagram.com/@");
}
#endif

namespace Lab {

void Task_PublishImageToGallery::execute()
{
#if CPPHTTPLIB_OPENSSL_SUPPORT
    TaskWithProgressBar::change_notification_when_execution_starts();

    auto const image_png_data =
        img::save_png_to_string(
            _image,
            stbiw_SaveOptions{
                .cancel_requested = [&]() { return cancel_requested(); },
                .set_progress     = [&](float progress) { set_progress(0.9f * progress); },
            }
        );
    if (!image_png_data.has_value())
    {
        _result = tl::make_unexpected("Failed to convert image to PNG, please try again."s);
        return;
    }
    auto cli = httplib::SSLClient{"api.cloudinary.com"};

    // Create the multipart/form-data request
    auto const items = httplib::MultipartFormDataItems{
        // Add the image file as a binary data item
        httplib::MultipartFormData{
            .name         = "file",
            .content      = *image_png_data,
            .filename     = "image.png",
            .content_type = "image/png",
        },
        httplib::MultipartFormData{
            .name         = "upload_preset",
            .content      = "gallery",
            .filename     = {},
            .content_type = {},
        },
        httplib::MultipartFormData{
            .name         = "tags",
            .content      = "gallery",
            .filename     = {},
            .content_type = {},
        },
        httplib::MultipartFormData{
            .name    = "context",
            .content = fmt::format(
                "title={}|description={}|author_name={}|author_link={}|email={}|agreed_to_have_it_shared_on_our_instagram={}",
                escape(_artwork_info.title),
                escape(_artwork_info.description),
                escape(_author_info.name),
                escape(process_link(_author_info.link)),
                escape(_legal_info.email),
                escape(_legal_info.has_agreed_to_share_on_instagram ? "true" : "false")
            ),
            .filename     = {},
            .content_type = {},
        },
    };

    // Send the POST request with the multipart/form-data
    auto const res = cli.Post("/v1_1/coollab/image/upload", items);
    if (!res || res->status != 200)
    {
        _result = tl::make_unexpected(
            res
                ? fmt::format("Failed to publish.\nStatus {}.\n{}: {}", res->status, res->reason, res->body)
                : fmt::format("Failed to publish.\n{}", httplib::to_string(res.error()))
        );
    }
#else
    assert(false && "Coollab was not built with the OpenSSL library because it was not found while compiling. You cannot use this function.");
    _result = tl::make_unexpected("This version of Coollab was not built with OpenSSL, you cannot publish images online."s);
#endif
}

auto Task_PublishImageToGallery::notification_after_execution_completes() const -> ImGuiNotify::Notification
{
    if (_result.has_value())
    {
        return {
            .type                 = ImGuiNotify::Type::Success,
            .title                = name(),
            .custom_imgui_content = []() {
                Cool::ImGuiExtras::markdown("You can now see your image online at [https://coollab-art.com/Gallery](https://coollab-art.com/Gallery)");
            },
        };
    }
    else // NOLINT(*else-after-return)
    {
        return {
            .type    = ImGuiNotify::Type::Error,
            .title   = name(),
            .content = _result.error(),
        };
    }
}

} // namespace Lab
