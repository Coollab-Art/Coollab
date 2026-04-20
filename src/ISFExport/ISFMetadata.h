#pragma once
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace Lab {

struct ISFInputFloat {
    std::optional<double> default_val{};
    std::optional<double> min_val{};
    std::optional<double> max_val{};
};

struct ISFInputPoint2D {
    std::optional<std::array<double, 2>> default_val{};
    std::optional<std::array<double, 2>> min_val{};
    std::optional<std::array<double, 2>> max_val{};
};

struct ISFInputColor {
    std::optional<std::array<double, 4>> default_val{}; // r, g, b, a
};

struct ISFInputBool {
    std::optional<bool> default_val{};
};

struct ISFInputLong {
    int                      default_val{0};
    std::vector<int>         values{};
    std::vector<std::string> labels{};
};

struct ISFInputImage {};

struct ISFInputAudio {
    std::optional<int> max{}; // MAX=1 means "compute volume" in our v2 convention
};

struct ISFInputAudioFFT {
    std::optional<int> max{};
};

using ISFInputData = std::variant<
    ISFInputFloat,
    ISFInputPoint2D,
    ISFInputColor,
    ISFInputBool,
    ISFInputLong,
    ISFInputImage,
    ISFInputAudio,
    ISFInputAudioFFT>;

struct ISFInput {
    std::string  name;
    ISFInputData data;
};

struct ISFPass {
    std::string target;
    bool        persistent{false};
};

struct ISFMetadata {
    std::vector<ISFInput>              inputs{};
    std::map<std::string, std::string> imported{}; // name -> relative file path
    std::vector<ISFPass>               passes{};

    auto to_json() const -> std::string;
};

} // namespace Lab
