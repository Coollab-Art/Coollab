#include "ModuleDependencies.h"
#include "Cool/Nodes/NodeId.h"
#include "Cool/String/String.h"
#include "Cool/TextureSource/TextureDescriptor.h"
#include "Nodes/Node.h"

namespace Lab {

static auto contains_two_or_more(std::string_view word, std::string_view text) -> bool
{
    auto const pos = Cool::String::find_word(word, text, 0);
    if (pos == std::string_view::npos)
        return false;

    auto const pos2 = Cool::String::find_word(word, text, pos + 1);
    return pos2 != std::string_view::npos;
}

void update_dependencies_from_shader_code(ModuleDependencies& dependencies, std::string shader_code)
{
    shader_code = Cool::String::remove_comments(shader_code);

    dependencies.time |= contains_two_or_more("_time", shader_code);
    dependencies.last_midi_button_pressed |= contains_two_or_more("_last_midi_button_pressed", shader_code)
                                             || contains_two_or_more("_last_last_midi_button_pressed", shader_code);
    dependencies.time_since_last_midi_button_pressed |= contains_two_or_more("_time_since_last_midi_button_pressed", shader_code);
    dependencies.audio_volume |= contains_two_or_more("_audio_volume", shader_code);
    dependencies.audio_waveform |= contains_two_or_more("_audio_waveform", shader_code);
    dependencies.audio_spectrum |= contains_two_or_more("_audio_spectrum", shader_code);
}

void update_dependencies_from_nodes_graph(ModuleDependencies& dependencies, Cool::NodesGraph const& graph, std::vector<Cool::NodeId> const& nodes_that_we_depend_on)
{
    dependencies.osc_channels.clear();
    dependencies.midi_channels.clear();

    for (auto const& node_id : nodes_that_we_depend_on)
    {
        graph.nodes().with_ref(node_id, [&](Cool::Node const& abstract_node) {
            auto const& node = abstract_node.downcast<Node>();
            for (auto const& value_input : node.value_inputs())
            {
                {
                    auto const* osc_channel = std::get_if<Cool::SharedVariable<Cool::OSCChannel>>(&value_input);
                    if (osc_channel)
                        dependencies.osc_channels.insert(osc_channel->value());
                }
                {
                    auto const* midi_channel = std::get_if<Cool::SharedVariable<Cool::MidiChannel>>(&value_input);
                    if (midi_channel)
                        dependencies.midi_channels.insert(midi_channel->value());
                }
                {
                    auto const* video_file = std::get_if<Cool::SharedVariable<Cool::TextureDescriptor_Video>>(&value_input);
                    if (video_file)
                        dependencies.time = true;
                }
                {
                    auto const* webcam = std::get_if<Cool::SharedVariable<Cool::TextureDescriptor_Webcam>>(&value_input);
                    if (webcam)
                        dependencies.always_rerender = true;
                }
                {
                    auto const* spout_syphon = std::get_if<Cool::SharedVariable<Cool::TextureDescriptor_SpoutSyphon>>(&value_input);
                    if (spout_syphon)
                        dependencies.always_rerender = true;
                }
            }
        });
    }
}

} // namespace Lab
