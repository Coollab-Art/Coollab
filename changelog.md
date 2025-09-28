# Changelog

## 1.7.0

- Removed the OpenGL backend, and replaced it with WebGPU! NB: this does **NOT** mean that Coollab is now a web application. WebGPU is just the name of the internal code library, which was originally designed for the web, but is now also available for desktop apps. So nothing changes for now, we are just using a more modern code library which allows a few things:
  - Particles on MacOS! 🥳🥳🥳
  - No more freeze when you add nodes to a big graph, the UI will still be responsive while the new visuals are being generated
  - It lays the first brick to run Coollab in the browser in the future (This is not a priority though, unless users really ask for it)

## 1.2.0 MacOS

- 💥 [**YOU NEED TO INSTALL THE LATEST VERSION OF THE LAUNCHER (2.0.0)**](https://coollab-art.com/Download). We had a nasty bug that could cause some projects to not appear properly in the launcher. We fixed it but this means all projects created with new versions of Coollab will not work well with the old launcher. We are sorry to add this annoying breaking change, but we decided to do it now while we don't have too many users, and while you don't have too many projects, rather than having the bug break some projects randomly in the future. 
- ✨ Coollab now works on MacOS!
- 🟦 Node: renamed "2D Variation (Gradient)" as "Make Displacement Map"
- 🟦 Node: renamed "Displace" as "Displacement Map" and improved it and added some parameters
![](https://github.com/user-attachments/assets/e0adaaef-fad8-496c-b8e3-7f2b6d6a6f88)
- 🟦 Node: added "Distance", that gives the distance to a Shape. You can use it for example to apply a Color Ramp that changes as you get away from the shape
![](https://github.com/user-attachments/assets/37929fcf-7215-45c6-b0fa-76de32ae63f5)
- 🟦 Node: better default values for "Fisheye" parameters
- 🤏 Improved fullscreen mode (<kbd>F11</kbd>). The window will no longer minify when clicking on another screen. It is also faster to switch between fullscreen and non-fullscreen, and the window does not flicker weirdly when switching to another window or screen
- 🤏 Added shortcuts <kbd>CTRL</kbd><kbd>+</kbd> and <kbd>CTRL</kbd><kbd>-</kbd> to zoom the UI
- 🤏 Renamed "UI Scale" as "UI Zoom"
- 🤏 *Save As* now switches to the new project by default. The setting to control this has been renamed as *\"Save As\" behaves as a \"Save Backup\"*
- 🤏 We now use the Dark theme by default, instead of using the theme configured in your OS (which was Light for most people)
- 🐛 When exporting an image that overwrites an existing image file, the backup project that we create will now also overwrite the existing backup project. And we added a warning to tell you that this will happen and ask for your confirmation
- 🐛 Notifications where not properly scaled when zooming the UI
- 🐛 We now stop checking for webcams info as soon as we stop using a webcam, which saves some performance

## 1.1.1 UI Scale

- ✨ Added inputs for Camera 3D's position and rotation
![](https://github.com/user-attachments/assets/435075c8-89ab-442f-82d5-5dd9ec44c06f)

## 1.1.0 UI Scale

- ✨ The UI will automatically scale to match your screen's pixel density (DPI)
- ✨ Added an option to scale the UI manually
![](https://github.com/user-attachments/assets/175414c0-8075-4f68-b524-2d1e15575a12)
- 🐛 Fixed: Settings were unnecessarily re-loaded from disk each time we saved them 
- 🐛 Fixed: On MacOS the user data where saved in the home directory (`~`) instead of `~/Library/Application Support`

## 1.0.1 Launcher

- ✨ If there is a problem (crash, etc.), errors will now be written to a file, so that you can then send it to us to help us debugging and fixing your problem. To find this file, [follow this short tutorial](https://coollab-art.com/Tutorials/Miscellaneous/Log%20File)
- ✨ Some messages that were displayed in the Console are now shown as Notifications, which should look nicer and be more noticeable
- 🐛 Fixed: the OSC listener did not start automatically when opening a project that was making use of OSC
- 🐛 Fixed: we used to always do some work in the background to check for the list of webcams, even if you were not using any webcam in your project
- 🤏 The framerate window now shows a plot of the performance over the last couple of frames

## 1.0.0 Launcher

### Launcher

- ✨ We are now using a launcher that will auto-install new Coollab versions automatically
- 💥 You can no longer open a project directly from Coollab, you have to go through the Launcher. (This is to ensure we use the right version of Coollab for each project, instead of facing an "incompatible version" error when trying to open it)
- ✨ Added a field to rename the project, in the center of the top menu bar
- ✨ Reworked how "Save As" works, to be more intuitive. You also have an option in the Settings menu to change its behaviour.

### Quality of Life changes

- ✨ Added shortcut <kbd>CTRL</kbd>+<kbd>E</kbd> to quickly export an image
- ✨ The aspect ratio of the View is now controlled through a small button in the View
![](https://github.com/user-attachments/assets/a536802b-9e06-40f7-bd57-7485d5a37859)
- 🤏 Merged Camera and Inspector windows. You can now edit camera values in the Inspector, when no node is selected.
![](https://github.com/user-attachments/assets/e7536282-3431-4f0b-97e7-dc3da13c8ca2)
- ✨ Removed some useless options in the Node's Inspector. Renaming a node is now done by right-clicking on it (or pressing <kbd>F2</kbd>)
![](https://github.com/user-attachments/assets/520b1999-61b5-4a49-9b08-33a23ecfac75)
- ✨ Added a progress bar for long operations like exporting an image, and it will no longer freeze Coollab while the operation is in progress
![](https://github.com/user-attachments/assets/421febbc-5c94-471e-93bf-0d7388f7f278)
- ✨ Added nice notifications for all kinds of events and errors
![](https://github.com/user-attachments/assets/cd43b632-0f90-492e-bbb4-356794bf5523)
- ✨ Every time value is now nicely formatted as "1h 27m 53s"

### Improved VJing setup

- ✨ The Output Window can now easily be turned fullscreen with <kbd>F10</kbd> or by right-clicking on it
- ✨ When opening the Output Window, the aspect ratio automatically adapts to it
- 🤏 We now automatically prevent your computer from going to sleep while you are exporting a video, or using the Output window to project Coollab during a live show or an installation
- 🤏 The Output Window now doesn't have a title bar
![](https://github.com/user-attachments/assets/73f3b250-8f33-4c9c-b602-63d5496027b5)

### New and improved nodes

- 🟦 Added a Glow node that works on any Image (the previous one only worked on Shapes, and is still here because it still looks better on Shapes than the new one). Thank you to [illtellyoulater](https://github.com/Coollab-Art/Coollab/issues/110) for this contribution!
![](https://github.com/user-attachments/assets/3337367d-d2ee-498b-a5ff-ba2131ecbedb)
- 🟦 Added "Monochrome" node
![](https://github.com/user-attachments/assets/08d7d280-c8b9-4ae2-b640-20d6c4817a9a)
- 🟦 Added "MIDI Multi-Select with Transition" node
![](https://github.com/Coollab-Art/Coollab/assets/45451201/bb809bdf-c0ae-44c1-af53-2ef3961d99fa)
- 🟦 "Mirror Repeat" and "Grid" nodes now have a "Content Size" parameter to allow you to fit your content in the square that will be repeated
![](https://github.com/user-attachments/assets/af5f70ee-d43c-488c-bc03-bc691d072386)
- 🟦 Image, Video, Webcam and Color Ramp are now using the "Mirror" repeat mode by default. This can be changed in the Inspector.
![](https://github.com/user-attachments/assets/117959ea-2b25-4fff-ba76-db739f3f090f)
- 🟦 Changed feedback loop node, you can now apply effects after the feedback node, that won't affect the image sent back into the loop (NB: this is still an experiment, this node will be greatly improved in the future)

### Miscellaneous

- 🤏 Webcams now open faster
- 🤏 Better error messages when the webcam doesn't work, and it is faster to re-open once the problem is fixed
- 🚧 You can now send http requests to Coollab to set values (NB: this is still a work in progress, we will document it later once it is stabilized)

### Bug fixes

- 🐛 Improved the linearity of our gradients from black to white
Before: (notice the huge black region on the left)
![Before](https://github.com/user-attachments/assets/6f6ef0e9-a67f-438a-8226-11d6b9e52330)
After:
![After](https://github.com/user-attachments/assets/a69dc220-b92a-46f4-af90-ea31ee642611)
- 🐛 Fix: our white was not a perfectly pure white in some cases
- 🐛 Fix: properly display accents (like "é") in webcam name
- 🐛 When a camera node was present but not actually linked into the graph, it still used the camera and forced us to rerender every frame. Now only the nodes that are actually used affect us.
- 🐛 Allow access to your microphone on MacOS for Audio nodes
- 🐛 Fix MacOS version crashing on startup
- 🐛 Many many other small bug fixes ^^

## 🐣 Beta 17

- ✨ Greatly improved video import: we now support videos with transparency, and GIFs
![](https://github.com/Coollab-Art/Coollab/assets/45451201/e5005eff-5b16-41ef-a4b1-74a46c481edc)
- ⚡ The performance of playing a video has been greatly improved
- 🐛 Video import now also works on Linux and MacOS
- 🐛 Many bug fixes around the video import and playback
- ✨ Improved MIDI support: buttons now have several modes (toggle, selector, pressed)
![](https://github.com/Coollab-Art/Coollab/assets/45451201/7826e34f-9210-4002-a145-385bf4107f29)
- 🟦 MIDI nodes: Added "Last MIDI button pressed", and you can now change the Min and Max output values of the "MIDI" node
- 🟦 Nodes: Added "Select" and "Multi-Select" which can typically be used in combination with MIDI buttons, to switch between various images / effects
![](https://github.com/Coollab-Art/Coollab/assets/45451201/c1692212-a6ea-4d2f-8e7d-4bc4f1c3f887)
- ✨ When exporting a video, if the output folder already contains frames from a previous export you now have 4 options, and by default we will prompt you to create a new folder
![](https://github.com/Coollab-Art/Coollab/assets/45451201/e5840829-21d5-4734-a388-5fd905dec17d)

## 🐣 Beta 16

- ✨ You can now import video files! Using the "Video from File" node.
![](https://github.com/Coollab-Art/Coollab/assets/45451201/fb7561df-f775-43d5-bbf4-1dedd32563bf)
- 🤏 The time in the timeline is now nicely formatted (as "3m 43s 512ms"). You can also input it like that (by CTRL+clicking on the timeline), as any combination of millisecond (ms) / second (s) / minute (m) / hour (h) / day (d) / week (w). For example "3m43".
- 🤏 The slider for some parameters (Zoom, Time Speed, etc.) now behaves logarithmically, meaning it will have equal precision in the 0-1 range as in 1-∞. Basically this means they are more practical to use.
- 🤏 When using a Drag widget, the mouse position now stays locked in place instead of wrapping around the screen.
- 🤏 Some node inputs now have a little info icon next to their pin, explaining what the parameter does in more details.
- 🤏 "Random" nodes now use an integer as a Seed, instead of any decimal number.
- 🤏 Improved Angle widget.
- 🤏 Improved final image size calculations based on desired aspect ratio.
- 🐛 Fixed image generation failure in some rare cases.
- 🐛 Fixed a crash when loading a project that was made in an older version of Coollab and that used a node that had been updated in Coollab.
- 🐛 Fix: Remove incompatible links when creating a link backward from a pin.

## 🐣 Beta 15

- ✨ Added a Time Speed on the timeline, which allows you to slow down or speed up your entire animation easily.
![](https://github.com/Coollab-Art/Coollab/assets/45451201/53cb9489-8924-4761-86bc-5191a00afa28)
- 🐛 Fixed camera movements when using particles
- 🐛 When dragging gizmos, mouse can now wrap around the screen
- 🐛 Fixed: When writing your own nodes, they were not detected.
- 🐛 Fixed a few nodes.
- 👩‍💻 The syntax for creating your own Coollab nodes is now 100% compatible with regular glsl syntax. You can now write structs, global variables, #define, etc.

## 🐣 Beta 14

- ✨ You can now create your own nodes! [Read the tutorial](https://coollab-art.com/Tutorials/Writing%20Nodes/Intro) to learn everything you need to know.
- 🐛 Fixed: Plugging a link into an input pin that already has a link now removes the old link. (This bug was introduced in the previous version).

## 🐣 Beta 13

- ✨ You can now copy-paste nodes! (CTRL+C / CTRL+V) You can even paste them from one project to another, or send them as text to a friend, who can then paste them in their instance of Coollab.
![](https://github.com/Coollab-Art/Coollab/assets/45451201/28a9c941-bc9a-49a6-b56f-5dadc2c005d7)
- ✨ Adding and removing nodes and links can now be undone and redone (CTRL+Z / CTRL+Y)
![](https://github.com/Coollab-Art/Coollab/assets/45451201/6e4d7e62-a919-4e71-81cf-76e5cb0b24e5)
- ✨ Changes to the 2D Camera can now be undone and redone (CTRL+Z / CTRL+Y)
- ✨ Applying a preset can now be undone and redone (CTRL+Z / CTRL+Y)

## 🐣 Beta 12

- ✨ Added OSC support! You can now control Coollab through OSC messages from your smartphone or another software on your computer! Just use the "OSC" node and select which channel's value you want to use.
![OSC](https://github.com/Coollab-Art/Coollab/assets/45451201/9e0b1284-97fd-4c4e-8ba5-0909ada8b242)
- ⚡ Improved performance when using Midi input.
- 🐛 Fixed a rare crash when using Midi input.
- 🐛 Fixed a bug that caused the Nodes window to zoom out a bit each time you opened Coollab.

## 🐣 Beta 11

- ✨ Added particles! This a very promising prototype that you can already use today. An important overhaul of the system will come at some point in a (probably distant) future. (NB: Unfortunately particles are not currently available on MacOS, and will not be for a very long while).
![Particles](https://github.com/Coollab-Art/Coollab/assets/45451201/7a8da2ef-df37-44fe-bf93-6cd37638c9f6)
- ✨ Improved the randomness of our Random and Noise nodes.
- ✨ Prevented mouse from getting blocked on the screen edges while dragging a widget or the camera. Instead the mouse wraps around.
- ✨ Prevented mouse from getting blocked on the screen edges while dragging a node or a link or a selection rectangle in the nodes view. Instead the canvas starts translating as expected.
- ✨ Greatly improved quite a few nodes, most notably "Glow".
- ✨ Added quite a few nodes, most notably "Blur" and "Adaptive Halftone".
- 🤏 You can now pan the Nodes view with the middle mouse button (and you can still use the right button as you used to).
- ✨ Added A4/A3/A2/A1 aspect ratio (when selecting the size of the View and the exported images).
- 🐛 Fixed crash on Windows when an image export took longer than 2 seconds.
- 🐛 Fixed freeze on Linux when exporting a video.
- 🐛 Fixed: we won't request use of your microphone unless we actually need it (e.g. as soon as you start using it as an audio input in Coollab).

## 🐣 Beta 10

- ✨ Added Audio support! You can now import an audio file and play it while you generate your images. You can also use its volume and waveform to control your images. You can also receive sound from an input device (microphone, etc.). Check out [our audio tutorial](https://coollab-art.com/Tutorials/Features/Audio) to learn everything about it!
![node_spectrum](https://github.com/Coollab-Art/Coollab/assets/45451201/17e77692-7941-4983-b2cf-a30b13aeee12)
- 🟦 Added new nodes that you can use to display the audio features: "Fill Function", "Function to Shape" and "Add Displacement".
- 💄 Added icons in the Commands menu.
![open_audio_config_command](https://github.com/Coollab-Art/Coollab/assets/45451201/3a49dd4b-1247-4329-8ed0-81004940ae77)
- 🐛 Fixed crash on Linux when opening a file explorer.
- 🐛 Temporary workaround: on Linux the history isn't saved when closing Coollab, in order to avoid a crash.
- 🚚 On Linux, moved user-data folder to *.local/share*

## 🐣 Beta 9

- ✨ Added MIDI support! You can now plug-in your MIDI keyboard, select your knob / slider by index, and use its value to control parameters of your nodes.
![image](https://github.com/Coollab-Art/.github/assets/45451201/5a8d4950-57a0-4282-b549-6c66487448c3)
- ✨ Added the "Paint" blend mode.

| ![image](https://github.com/Coollab-Art/.github/assets/45451201/6f57a43d-a422-4056-81e1-c691e4c85d84) | ![image](https://github.com/Coollab-Art/.github/assets/45451201/9b12b5bf-7f15-408b-93e9-552e0caa30ea) |
| ----------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------- |
| "Light" mode (what was already there in Coollab)                                                      | "Paint" mode (the new blend mode)                                                                     |

- ✨ Added an Output window that you can project during live shows, while still having your View window on your screen to move the camera and the widgets. To open this window, go in the `Commands` menu and select `Open output window`.
![image](https://github.com/Coollab-Art/.github/assets/45451201/3d6a14ec-69ca-44b1-81e0-1d4139b72544)
- ✨ Added "Open Backup" in case you accidentally refused to save your unsaved changes.
![image](https://github.com/Coollab-Art/.github/assets/45451201/99287d7b-15a1-480b-98d4-0ca5c4777b86)
- 🐛 Fix: the transparency information was sometimes getting lost between nodes.
- 🐛 Fix: crash on Linux "Too many open files".
- ⚡️ Fix lag when editing the color gradient on a Color Ramp node.
- 👩‍💻 Replaced CIELAB color space with Oklab
- 👩‍💻 Replaced HSLuv color space with Okhsl

## 🐣 Beta 8

- ✨ Added project files: you can now save and open projects, allowing you to keep and share your work!
![image](https://github.com/Coollab-Art/Coollab/assets/45451201/00270343-3a45-4e92-93cc-729f0f674c1e)
- ✨ Coollab now has an installer! You don't need to download the raw executable anymore.
![image](https://github.com/Coollab-Art/Coollab/assets/45451201/a1042659-e003-4dcf-b917-79505c84c28e)
- ✨ Added the "Webcam" node!
![image](https://github.com/Coollab-Art/Coollab/assets/45451201/77fd1a74-2e7c-43eb-914a-90a711ce2cae)
- ✨ Added feedback loops! Check out the "Feedback" node.
![ezgif-5-a3c56b1c92](https://github.com/Coollab-Art/Coollab/assets/45451201/d6513535-6f53-4932-b260-20a54c032380)
- 🚚 Renamed "Space Transformation" category as "2D Modifier".
- 🐛 Fix: some nodes had the wrong color.
- 🐛 Fix: on Linux, for some window managers like i3, the context menu was not behaving properly. (Now by default we disable multi-viewport in you use one of these custom window managers, which fixes the issue but prevents you from dragging windows outside of the main Coollab window. This can be changed in the Settings menu.) 

## 🐣 Beta 7

- ✨ Only one camera (either 2D or 3D) can be active at the same time.
- ✨ Improved "Distortion Map" node.
- ✨ Added several "3D Shape from 2D" nodes.
- 🚚 Renamed "Boolean" categories as "Blend".
- 🚚 Renamed "Blend" nodes as "Mix".
- 🚚 Moved "Time" node to the "Input" category.
- 🐛 Fixed implicit conversion between numbers and angles: a number equal to 1 (white) now corresponds to a full turn (360 degrees).

## 🐣 Beta 6

- ✨ Added many Shape Booleans.
- ⚡️ Big performance improvements. If you experienced lag spikes before, they should be gone now!
- 🐛 Fix: some nodes had the wrong color.

## 🐣 Beta 5

- ✨ Added gizmos on the view that allow you to edit Point2Ds visually.
![Animation](https://github.com/Coollab-Art/Coollab/assets/45451201/b2a5ad91-e9bf-42d6-ab7a-c60e86e97c40)

- ✨ Added Bezier Curve node.
- ✨ Added a full 3D renderer node.
![Animation-min](https://github.com/Coollab-Art/Coollab/assets/45451201/5996fb72-258b-46ff-b87d-4195bda21215)

- ✨ Added tips that will show up from time to time and teach you about the subtleties and shortcuts of Coollab.
![image](https://github.com/Coollab-Art/Coollab/assets/45451201/112c4431-5e68-4617-9bcb-6591755aae05)

- ✨ Main input pins now have a different icon to distinguish them from the other pins.
![image](https://github.com/Coollab-Art/Coollab/assets/45451201/73191129-3c1a-4fb0-afa8-6651100399aa)

- 🐛 Fix: prevent panning the nodes while you are panning the camera.
- 🐛 Fixed the camera controls not working when the View window was on another screen.

## 🐣 Beta 4

- ✨ Improved quite a few existing nodes.
- ✨ Added new nodes. Check out our "Cloud" renderer for 3D Shapes!
- ✨ Node pins now have a color that reflects the kind of node that you can plug into them. (e.g. a pin that wants a Shape 3D will be yellow, just like the Shape 3D nodes.)
- ✨ Smarter automatic main node selection.
- ✨ All nodes can now be viewed by themselves, without requiring a "renderer" node (which used to be the case for Curves and 3D Shapes).
- ✨ Added buttons on the view to freeze / enable the 2D and 3D cameras.
- 🐛 Fixed some effects (Space Transformations) not being applied on top of 3D shapes.

## 🐣 Beta 3

- ✨ Added many 3D nodes.
- ✨ Added a Frame (Comment) node.
- ✨ In the "Export" menu, added a button to share your image online in [Coollab's Gallery](https://coollab-art.com/Gallery).
- 💄 Completely overhauled UI.
- 💄 New logo.
- 💄 Added Settings to change your color theme.
- 💄 Improved the Dark color theme.
- 💄 Added a Light color theme.
- 💄 Added an option to use the color theme set by your OS (Dark or Light).
- 💄 The View now uses a fixed aspect ratio by default (you can change this in the Preview menu).
- 💄 Improved Cameras window + added an option to lock one of the two cameras (2D or 3D) when using both 2D and 3D nodes.
- 🐛 Fixed the camera controls when using a fixed aspect ratio or fixed number of pixels.
- 🐛 Fix: the nodes categories were not sorted on MacOS.
- 🐛 Fix: could not place a node that had the same name as its category.
- 👩‍💻 Node files: now support the `#define` macro, just like any glsl file.
- 👩‍💻 Node files: added boolean and matrices types for INPUTs, main function and helper functions (they can be used anywhere like any other type now).

## 🐣 Beta 2

- Fix "Reserved built-in name" error on some GPUs
- Fix: properly display error messages when loading (parsing) a node failed (very useful for people creating new nodes)
- Started using icons all over the place (window titles, buttons, menus, ...)
- Many more nodes, and improvements to existing nodes

## 🐣 Beta 1

- Fix the huge visual glitches that appeared on some computers
- Image Node: fix the ghost error messages that it would create and that couldn't be removed
- Nodes:
  - New nodes
  - Fixes
  - Improvements
  - Added presets for some of them
  - Reorganized categories a bit
- All shapes are now nicely anti-aliased
- Added the debug options menu to the released executables
- Added debug option to get an info dump in order to help developers debug

## 🐣 Beta 0

- First version
