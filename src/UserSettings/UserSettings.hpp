#pragma once
#include "Cool/Serialization/Json.hpp"
#include "Cool/Serialization/JsonAutoSerializer.hpp"

namespace Lab {

struct UserSettings {
    bool save_as_behaves_as_save_backup{false};

    auto imgui() -> bool;
    auto imgui_save_as_behaves_as_save_backup() -> bool;

    void update() { _serializer.update(); }
    void save() { _serializer.save(); }

private:
    // Must be declared last, after all the variables it serializes, so that the values it loads overwrite the default values, and not the other way around
    Cool::JsonAutoSerializer _serializer{
        "coollab_user_settings.json",
        false /*autosave_when_destroyed*/, // This is a static instance, so saving it in the destructor is dangerous because we don't know when it will happen exactly. Instead, we call save manually at the end of the run()
        [&](nlohmann::json const& json) {
            Cool::json_get(json, "Save As behaves as Save Backup", save_as_behaves_as_save_backup);
        },
        [&](nlohmann::json& json) {
            Cool::json_set(json, "Save As behaves as Save Backup", save_as_behaves_as_save_backup);
        },
        false /*use_shared_user_data*/
    };
};

inline auto user_settings() -> UserSettings&
{
    static UserSettings instance;
    return instance;
}

} // namespace Lab