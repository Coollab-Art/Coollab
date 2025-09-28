#include "LegalInfo.h"
#include <imgui.h>
#include <optional>
#include "Cool/ImGui/ImGuiExtras.h"
#include "Cool/ImGui/markdown.h"

namespace Lab {

auto LegalInfo::imgui() -> bool
{
    bool b{false};
    b |= ImGui::InputText("Email *", &email);
    Cool::ImGuiExtras::help_marker("We will not use this data. This is only so that you can email us and request to edit or delete your data or artwork. We will only accept requests to edit an artwork if it is sent from the email that you provide here. Send all your requests at coollab.lib@gmail.com");

    ImGui::Checkbox("I agree that my artwork will be publicly visible on", &has_agreed_to_share_on_gallery);
    ImGui::SameLine();
    Cool::ImGuiExtras::markdown(" [Coollab's gallery](https://coollab-art.com/Gallery).");
    Cool::ImGuiExtras::help_marker("We will keep the artwork for 5 years (or until you request us to remove it). After that we will remove the artwork and all associated data from our database.");

    ImGui::Checkbox("I additionally agree to have my artwork shared on", &has_agreed_to_share_on_instagram);
    ImGui::SameLine();
    Cool::ImGuiExtras::markdown(" [Coollab's Instagram](https://www.instagram.com/coollab_art/).");
    Cool::ImGuiExtras::help_marker("This is optional. If you don't want to, please at least consider sharing it on your account and tagging us @coollab_art and using the #coollab_art.");

    return b;
}

auto LegalInfo::missing_information_message() const -> std::optional<std::string>
{
    if (!has_agreed_to_share_on_gallery)
        return "You must agree to share your artwork on Coollab's gallery. See the legal section above.";
    if (email.empty())
        return "You must provide an email so that you can contact us to request changes to your data. See the legal section above.";

    return std::nullopt;
}

} // namespace Lab