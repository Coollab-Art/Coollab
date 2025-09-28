#pragma once
#include "Cool/Serialization/Json.hpp"

namespace Lab {

struct ArtworkInfo {
    std::string title{};
    std::string description{};

    auto imgui() -> bool;
};

inline void to_json(nlohmann::json& json, ArtworkInfo const& info)
{
    Cool::json_set(json, "Title", info.title);
    Cool::json_set(json, "Description", info.description);
}

inline void from_json(nlohmann::json const& json, ArtworkInfo& info)
{
    Cool::json_get(json, "Title", info.title);
    Cool::json_get(json, "Description", info.description);
}

} // namespace Lab