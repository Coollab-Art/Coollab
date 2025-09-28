#pragma once
#include "ArtworkInfo.h"
#include "AuthorInfo.h"
#include "Cool/Task/TaskWithProgressBar.hpp"
#include "LegalInfo.h"
#include "no_sleep/no_sleep.hpp"

namespace Lab {

class Task_PublishImageToGallery : public Cool::TaskWithProgressBar {
public:
    Task_PublishImageToGallery(ArtworkInfo artwork_info, AuthorInfo author_info, LegalInfo legal_info, img::Image image)
        : _artwork_info{std::move(artwork_info)}
        , _author_info{std::move(author_info)}
        , _legal_info{std::move(legal_info)}
        , _image{std::move(image)}
    {}

    auto name() const -> std::string override { return fmt::format("Publishing image \"{}\"", _artwork_info.title); }

private:
    void execute() override;
    auto needs_user_confirmation_to_cancel_when_closing_app() const -> bool override { return true; }
    auto notification_after_execution_completes() const -> ImGuiNotify::Notification override;

private:
    ArtworkInfo _artwork_info;
    AuthorInfo  _author_info;
    LegalInfo   _legal_info;
    img::Image  _image;

    no_sleep::Scoped                _disable_sleep{COOL_APP_NAME, COOL_APP_NAME " is publishing an image online", no_sleep::Mode::ScreenCanTurnOffButKeepComputing};
    tl::expected<void, std::string> _result;
};

} // namespace Lab