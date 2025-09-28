#pragma once
#include <optional>
#include "Cool/Serialization/Json.hpp"

namespace Lab {

struct LegalInfo {
    std::string email{};
    bool        has_agreed_to_share_on_gallery{false};
    bool        has_agreed_to_share_on_instagram{false};

    void reset_agreement()
    {
        has_agreed_to_share_on_gallery   = false;
        has_agreed_to_share_on_instagram = false;
    }

    auto               imgui() -> bool;
    [[nodiscard]] auto missing_information_message() const -> std::optional<std::string>;
};

inline void to_json(nlohmann::json& json, LegalInfo const& info)
{
    Cool::json_set(json, "Email", info.email);
}

inline void from_json(nlohmann::json const& json, LegalInfo& info)
{
    Cool::json_get(json, "Email", info.email);
}

} // namespace Lab