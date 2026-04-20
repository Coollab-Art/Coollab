#include "ISFMetadata.h"
#include <fmt/core.h>
#include <sstream>

namespace Lab {

static auto escape_json_string(std::string const& s) -> std::string
{
    std::string result;
    result.reserve(s.size());
    for (char c : s)
    {
        switch (c)
        {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default: result += c; break;
        }
    }
    return result;
}

static auto format_double(double v) -> std::string
{
    // Remove trailing zeros but keep at least one decimal
    auto s = fmt::format("{}", v);
    if (s.find('.') == std::string::npos)
        s += ".0";
    return s;
}

static auto input_data_to_json(ISFInputData const& data) -> std::string
{
    return std::visit([](auto&& d) -> std::string {
        using T = std::decay_t<decltype(d)>;

        if constexpr (std::is_same_v<T, ISFInputFloat>)
        {
            std::string result = "\"TYPE\": \"float\"";
            if (d.default_val)
                result += fmt::format(", \"DEFAULT\": {}", format_double(*d.default_val));
            if (d.min_val)
                result += fmt::format(", \"MIN\": {}", format_double(*d.min_val));
            if (d.max_val)
                result += fmt::format(", \"MAX\": {}", format_double(*d.max_val));
            return result;
        }
        else if constexpr (std::is_same_v<T, ISFInputPoint2D>)
        {
            std::string result = "\"TYPE\": \"point2D\"";
            if (d.default_val)
                result += fmt::format(", \"DEFAULT\": [{}, {}]", format_double((*d.default_val)[0]), format_double((*d.default_val)[1]));
            if (d.min_val)
                result += fmt::format(", \"MIN\": [{}, {}]", format_double((*d.min_val)[0]), format_double((*d.min_val)[1]));
            if (d.max_val)
                result += fmt::format(", \"MAX\": [{}, {}]", format_double((*d.max_val)[0]), format_double((*d.max_val)[1]));
            return result;
        }
        else if constexpr (std::is_same_v<T, ISFInputColor>)
        {
            std::string result = "\"TYPE\": \"color\"";
            if (d.default_val)
                result += fmt::format(", \"DEFAULT\": [{}, {}, {}, {}]",
                                      format_double((*d.default_val)[0]),
                                      format_double((*d.default_val)[1]),
                                      format_double((*d.default_val)[2]),
                                      format_double((*d.default_val)[3]));
            return result;
        }
        else if constexpr (std::is_same_v<T, ISFInputBool>)
        {
            std::string result = "\"TYPE\": \"bool\"";
            if (d.default_val)
                result += fmt::format(", \"DEFAULT\": {}", *d.default_val ? "true" : "false");
            return result;
        }
        else if constexpr (std::is_same_v<T, ISFInputLong>)
        {
            std::string result = fmt::format("\"TYPE\": \"long\", \"DEFAULT\": {}", d.default_val);
            if (!d.values.empty())
            {
                result += ", \"VALUES\": [";
                for (size_t i = 0; i < d.values.size(); ++i)
                {
                    result += fmt::format("{}", d.values[i]);
                    if (i + 1 < d.values.size())
                        result += ", ";
                }
                result += "]";
            }
            if (!d.labels.empty())
            {
                result += ", \"LABELS\": [";
                for (size_t i = 0; i < d.labels.size(); ++i)
                {
                    result += fmt::format("\"{}\"", escape_json_string(d.labels[i]));
                    if (i + 1 < d.labels.size())
                        result += ", ";
                }
                result += "]";
            }
            return result;
        }
        else if constexpr (std::is_same_v<T, ISFInputImage>)
        {
            return "\"TYPE\": \"image\"";
        }
        else if constexpr (std::is_same_v<T, ISFInputAudio>)
        {
            std::string result = "\"TYPE\": \"audio\"";
            if (d.max)
                result += fmt::format(", \"MAX\": {}", *d.max);
            return result;
        }
        else if constexpr (std::is_same_v<T, ISFInputAudioFFT>)
        {
            std::string result = "\"TYPE\": \"audioFFT\"";
            if (d.max)
                result += fmt::format(", \"MAX\": {}", *d.max);
            return result;
        }
    },
                      data);
}

auto ISFMetadata::to_json() const -> std::string
{
    std::stringstream ss;
    ss << "/*{\n";
    ss << "    \"ISFVSN\": \"2\"";

    // INPUTS
    if (!inputs.empty())
    {
        ss << ",\n    \"INPUTS\": [\n";
        for (size_t i = 0; i < inputs.size(); ++i)
        {
            ss << "        { \"NAME\": \"" << escape_json_string(inputs[i].name) << "\", ";
            ss << input_data_to_json(inputs[i].data);
            ss << " }";
            if (i + 1 < inputs.size())
                ss << ",";
            ss << "\n";
        }
        ss << "    ]";
    }

    // IMPORTED
    if (!imported.empty())
    {
        ss << ",\n    \"IMPORTED\": {\n";
        size_t i = 0;
        for (auto const& [name, path] : imported)
        {
            ss << "        \"" << escape_json_string(name) << "\": { \"PATH\": \"" << escape_json_string(path) << "\" }";
            if (i + 1 < imported.size())
                ss << ",";
            ss << "\n";
            ++i;
        }
        ss << "    }";
    }

    // PASSES
    if (!passes.empty())
    {
        ss << ",\n    \"PASSES\": [\n";
        for (size_t i = 0; i < passes.size(); ++i)
        {
            if (passes[i].target.empty())
            {
                ss << "        { }"; // Final pass renders to screen
            }
            else
            {
                ss << "        { \"TARGET\": \"" << escape_json_string(passes[i].target) << "\"";
                if (passes[i].persistent)
                    ss << ", \"PERSISTENT\": true";
                ss << " }";
            }
            if (i + 1 < passes.size())
                ss << ",";
            ss << "\n";
        }
        ss << "    ]";
    }

    ss << "\n}*/\n";
    return ss.str();
}

} // namespace Lab
