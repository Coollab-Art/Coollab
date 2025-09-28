#include "Tips.h"
#include "Cool/ImGui/IcoMoonCodepoints.h"
#include "Cool/Input/CTRL_OR_CMD.hpp"

namespace Lab {

auto all_tips() -> Cool::Tips
{
    static constexpr auto tips = std::array{
        "Our [Youtube tutorials](https://youtu.be/FK1sMPgcE4Q?list=PLuMMMaL5kN3mG6fSeSYTqC-sc_OTX3unQ) can help you get started with Coollab.",
        "In the \" " ICOMOON_TREE "Nodes\" window, right-click (or press A) to create your first node!", // NOLINT(bugprone-suspicious-missing-comma)
        "You can press ESCAPE to close these popups.",
        "You can change the color theme in the \" " ICOMOON_COG "Settings\" menu.",
        "Right-click on a node to replace it with a new node of the same category.",
        "Use " CTRL_OR_CMD "+E to quickly export an image with the current export settings",
        "Use F in the \" " ICOMOON_TREE "Nodes\" window to focus the view on the nodes.",
        CTRL_OR_CMD "+click on a slider to input a value manually. (It also allows you to bypass the bounds of the slider!).",
        "In an angle or direction widget, hold SHIFT to snap to precise angles.",
        "Press ALT while scrolling in the \" " ICOMOON_IMAGE "View\" to zoom relative to the center of the view, instead of relative to the position of the mouse.",
        "You can rotate the 2D camera by holding SHIFT and scrolling the mouse wheel over the \" " ICOMOON_IMAGE "View\".",
        "Right-click on a widget to edit its metadata (range of a slider, etc.).",
        "You can save your own presets for a node if you like the parameters (and metadata) you found. You will find this option at the bottom of the the \" " ICOMOON_EQUALIZER "Inspector\" when a node is selected.",
        "You can copy your nodes (" CTRL_OR_CMD "+C) and send them as text to a friend, who can then paste them in their instance of Coollab (" CTRL_OR_CMD "+V). You can also paste them in another project of yours.",
        "When copying nodes, you can press SHIFT to copy links going into the copied nodes, even if the link is starting from a node that is not copied.", // This one would really need a gif to explain it more clearly.
        "The undo-history (" CTRL_OR_CMD "+Z / " CTRL_OR_CMD "+Y) is preserved even when you close and re-open Coollab! You can control its size in the \" " ICOMOON_COG "Settings\" menu.",
        R"(You can create your own color themes by going in the "Debug" menu (on the right of the menu bar) and selecting "Color Themes: Editor".)",

        // Bonus ImGui tips
        "Drag from the window menu button (the arrow button in the upper-right corner) to move all tabs at once.",
        "You can hold SHIFT to disable docking. Useful when you try to move a window around and docking gets in your way.",
        "Double-click on the lower-right corner of a floating window to fit its size to its content.",
        "Use TAB and SHIFT+TAB to cycle through the keyboard-editable fields of a window.",
        R"STR(Here are a few useful shortcuts:
  * While inputting text:
    * )STR" CTRL_OR_CMD R"STR(+Left/Right to word jump.
    * )STR" CTRL_OR_CMD R"STR(+A or double-click to select all. 
    * )STR" CTRL_OR_CMD R"STR(+X/C/V to use clipboard cut/copy/paste. 
    * )STR" CTRL_OR_CMD R"STR(+Z, )STR" CTRL_OR_CMD R"STR(+Y to undo/redo. 
    * ESCAPE to revert.
  * To navigate with the keyboard:
    * )STR" CTRL_OR_CMD R"STR(+TAB to select a window.
    * Arrow keys to navigate inside a window.
    * ENTER to activate a widget.
    * ESCAPE to deactivate a widget, close popup, exit child window.
    * ALT to jump to the menu of a window.
)STR",
    };

    return tips;
}

} // namespace Lab