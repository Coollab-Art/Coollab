#pragma once
#include <Cool/Dependencies/AnySharedVariable.h>
#include <Cool/Nodes/Pin.h>
#include <Cool/Variables/AnyVariable.h>
#include <ser20/types/optional.hpp>
#include "Cool/Nodes/NodeDefinitionIdentifier.h"

namespace Lab {

struct NodePodPart {
    Cool::NodeDefinitionIdentifier id_names{};
    std::string                    name{};

    std::vector<Cool::InputPin>  input_pins{};
    std::vector<Cool::OutputPin> output_pins{};
    size_t                       number_of_main_input_pins{};
    size_t                       number_of_function_inputs{};

    std::optional<size_t> particles_count{}; // HACK Only used in the case of a Particle Initializer node

private:
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("Name", name),
            ser20::make_nvp("Definition", id_names),
            ser20::make_nvp("Input Pins", input_pins),
            ser20::make_nvp("Output Pins", output_pins),
            ser20::make_nvp("Number of main input pins", number_of_main_input_pins),
            ser20::make_nvp("Number of function inputs", number_of_function_inputs),
            ser20::make_nvp("Particle count", particles_count)
        );
    }
};

/// POD representation of a node. Can be copied without copying any reference.
struct NodeAsPOD {
    NodePodPart                    pod_part{};
    std::vector<Cool::AnyVariable> value_inputs{};

private:
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("POD part", pod_part),
            ser20::make_nvp("Value inputs", value_inputs)
        );
    }
};

class Node {
public:
    Node() = default;
    Node(Cool::NodeDefinitionIdentifier const& id_names, size_t number_of_main_input_pins, size_t number_of_function_inputs)
        : _d{
            .id_names                  = id_names,
            .number_of_main_input_pins = number_of_main_input_pins,
            .number_of_function_inputs = number_of_function_inputs,
        }
    {}
    explicit Node(NodePodPart const& pod_part)
        : _d{pod_part}
    {}

    auto name() const -> std::string { return _d.name; }
    void set_name(std::string const& name) { _d.name = name; }
    auto name_ref() -> std::string& { return _d.name; }

    auto definition_name() const -> std::string { return _d.id_names.definition_name; }
    auto category_name() const -> std::string { return _d.id_names.category_name; }

    auto id_names() const -> Cool::NodeDefinitionIdentifier { return _d.id_names; }

    [[nodiscard]] auto input_pins() const -> auto const& { return _d.input_pins; }
    [[nodiscard]] auto input_pins() -> auto& { return _d.input_pins; }
    [[nodiscard]] auto output_pins() const -> auto const& { return _d.output_pins; }
    [[nodiscard]] auto output_pins() -> auto& { return _d.output_pins; }
    [[nodiscard]] auto value_inputs() -> auto& { return _value_inputs; }
    [[nodiscard]] auto value_inputs() const -> auto const& { return _value_inputs; }

    auto number_of_main_input_pins() const -> size_t { return _d.number_of_main_input_pins; }
    auto main_input_pin(size_t main_input_index) const -> Cool::InputPin const&
    {
        assert(main_input_index < number_of_main_input_pins());
        return input_pins()[main_input_index];
    }
    auto main_output_pin() const -> Cool::OutputPin const& { return output_pins()[0]; }
    /// This corresponds to a subset of all the input_pints(); the ones that correspond to an INPUT of the node.
    auto pin_of_function_input(size_t function_input_index) const -> Cool::InputPin const& { return input_pins()[function_input_pin_idx_begin() + function_input_index]; }
    auto pin_of_value_input(size_t value_input_index) const -> Cool::InputPin const& { return input_pins()[value_input_pin_idx_begin() + value_input_index]; }
    auto pin_of_output_index(size_t output_index_index) const -> Cool::OutputPin const& { return output_pins()[1 + output_index_index]; }

    auto main_input_pin_idx_begin() const -> size_t { return 0; }
    auto main_input_pin_idx_end() const -> size_t { return _d.number_of_main_input_pins; }
    auto function_input_pin_idx_begin() const -> size_t { return main_input_pin_idx_end(); }
    auto function_input_pin_idx_end() const -> size_t { return function_input_pin_idx_begin() + _d.number_of_function_inputs; }
    auto value_input_pin_idx_begin() const -> size_t { return function_input_pin_idx_end(); }
    auto value_input_pin_idx_end() const -> size_t { return _d.input_pins.size(); }

    auto is_audio_node() const -> bool;

    auto particles_count() -> std::optional<size_t>& { return _d.particles_count; }
    auto particles_count() const -> std::optional<size_t> { return _d.particles_count; }
    void set_particles_count(std::optional<size_t> particles_count) { _d.particles_count = particles_count; }

    auto as_pod() const -> NodeAsPOD;

    void clear_all_error_messages();

private:
    NodePodPart                          _d{};
    std::vector<Cool::AnySharedVariable> _value_inputs;
    // NB: when adding data to Node, add it to NodePodPart if it is some pod data. Otherwise add it here but you then also need to add it to NodeAsPOD too, and to as_pod(), and to the constructor that takes NodeAsPOD, and to paste_nodes() of NodesConfig.

private:
    friend class ser20::access;
    template<class Archive>
    void serialize(Archive& archive)
    {
        archive(
            ser20::make_nvp("POD part", _d),
            ser20::make_nvp("Value inputs", _value_inputs)
        );
    }
};

auto to_string(Node const& node) -> std::string;

} // namespace Lab
