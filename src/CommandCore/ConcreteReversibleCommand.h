#pragma once

#include "CommandCore/ReversibleCommand.h"

namespace Lab {

template<typename T>
concept ConcreteReversibleCommand = requires(const T& a, const T& b, CommandExecutionContext_Ref& command_execution_context) {
    // Has an execute() method
    a.execute(command_execution_context);
    // Has a revert() method
    a.revert(command_execution_context);
    // Has a to_string() method
    {
        a.to_string()
    }
    -> std::convertible_to<std::string>;
    // Has a merge() method
    a.merge(b); // -> std::convertible_to<std::optional<T>> // This convertible_to directive doesn't compile :'(

    // TODO require a serialization function
};

} // namespace Lab
