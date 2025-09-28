#pragma once
#include "Cool/Serialization/Json.hpp"

namespace Lab {

struct AuthorInfo {
    std::string name{};
    std::string link{};

    auto imgui() -> bool;
};

inline void to_json(nlohmann::json& json, AuthorInfo const& info)
{
    Cool::json_set(json, "Name", info.name);
    Cool::json_set(json, "Link", info.link);
}

inline void from_json(nlohmann::json const& json, AuthorInfo& info)
{
    Cool::json_get(json, "Name", info.name);
    Cool::json_get(json, "Link", info.link);
}

} // namespace Lab