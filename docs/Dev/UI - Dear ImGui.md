

### Our UI library, Dear ImGui

#### The Demo Window

The Demo window is a very useful tool that will show you everything that is possible to do with *Dear ImGui*. It can be opened from the "Debug" menu:

![toggle_imgui_demo_window](https://github.com/user-attachments/assets/570cb30a-40e6-4d9a-bf4f-6a030f1a8a49)

#### The Item Picker

If you want to see the code you need to call to create a given widget, use the **"Item Picker"** from the Demo window. Just click on any widget and it will take you to the corresponding piece of code in your IDE! (Make sure you are launching your programmer with a debugger, otherwise it will just crash)

![](./img/imgui_item_picker.png)

#### Important *Dear ImGui* functions

##### `Begin()` / `End()` to create a window

##### `SameLine()`

##### `Separator()`

##### `PushID()` / `PopID()`

See [*Dear ImGui's FAQ about IDs*](https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-about-the-id-stack-system).

#### ImGui Extras

We have quite a few additional custom widgets available in [`<Cool/ImGui/ImGuiExtras.h>`](https://github.com/Coollab-Art/Cool/blob/main/src/Cool/ImGui/ImGuiExtras.h).
