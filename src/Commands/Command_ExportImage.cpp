#include "Command_ExportImage.h"
#include "App.h"

namespace Lab {

void Command_ExportImage::execute(CommandExecutionContext_Ref const& ctx) const
{
    ctx.app().image_export(image_params, autosave, override);
}

} // namespace Lab

#include "CommandCore/LAB_REGISTER_COMMAND.h"

LAB_REGISTER_COMMAND(Lab::Command_ExportImage)