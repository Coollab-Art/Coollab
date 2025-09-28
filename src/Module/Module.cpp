#include "Module.h"
#include "Cool/Log/message_console.hpp"

namespace Lab {

void Module::log_module_error(tl::expected<void, Cool::ErrorMessage> const& result, Cool::MessageId& message_id) const
{
    if (result.has_value())
    {
        Cool::message_console().remove(message_id);
    }
    else
    {
        Cool::message_console().send(
            message_id,
            {
                .type               = Cool::MessageType::Error,
                .title              = name(),
                .content            = result.error().message,
                .clipboard_contents = result.error().clipboard_contents,
            }
        );
    }
}

auto Module::needs_to_rerender() const -> bool
{
    return _needs_to_rerender_flag.is_dirty() || std::any_of(_modules_that_we_depend_on.begin(), _modules_that_we_depend_on.end(), [&](auto&& module) {
               return module->needs_to_rerender();
           });
};

} // namespace Lab