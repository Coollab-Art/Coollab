Small, simple
Mostly 2D

Coollab IS:
- A powerful and easy-to-use **VJing** software
- A way to create **interactive visuals** (audio-reactive, webcam, midi, osc, etc.) -> need to support mouse and keyboard, especially for export as website background, following mouse is nice (with some damping, might be applied directly on the Mouse, but might be better to have something generic in the data-processing graph (get mouse -> damping -> set variable in the blackboard) (need to be able to preview that Point2D on screen, to check if our effect works as intended (though not really needed because you can always draw a circle at this position manually with the Visual graph)))
- A way to create effects and transitions for your OBS stream, your website, etc.
<!-- - A **procedural texture** generator (NB: it would be nice to be able to import 3D models with their UVs and some how the texture would look on them / generate a texture based on 3D attributes and then generate a texture that maps properly on the UVs) --> Naah we are not Z-brush
- A 2D Texture and Pattern generator
- A **compositing**, **post-processing** and **editing** tool -> need to support multiple audios, and basic transitions like cross-fade
- A **2D animation** tool
- Has basic 3D features
- A 2D-focused tool (with light 3D features)
- A high-level **shader editor**
- A nice environment to write your own **glsl shaders**

Coollab is NOT REALLY (but you might still use it as):
- A live-coding environment
- A procedural texture generator for 3D objects (like z-brush)

Coollab IS NOT:
- ~~A 3D modelling software~~
- ~~A game engine~~
- ~~A powerful 3D renderer~~
- ~~A painting software (like Krita)~~
- ~~A vector-based graphics tool~~


#### Easy to use

We can't produce as many cool features as big softwares, so we should embrace our minimalistic essence and produce few but polished and intuitive features. Our tool might only be useful for a few use cases, but we want it to be the best for those use cases.

#### Prefer Visual Quality to Performance

We are not building a game engine; even though performance is important because we want to be able to visualize our changes in real time, visual quality is and should remain our main focus.

#### Be cross-platform

Refuse any solution that would reduce accessibility to the software, especially when cross-platform solutions exist.

Examples of such technologies are Cuda (Nvidia only, can be replaced by Vulkan's compute shaders) and OpenGL 4.0 and greater (not suported on MacOS, can be replaced by Vulkan or WebGPU).

#### Build great debug tools

They will help you a lot later down the road. And they can also be helpful to users, so keep them in the final build: for example to view the history, or the registries.

#### Easy to use

We can't produce as many cool features as big softwares, so we should embrace our minimalistic essence and produce few but polished and intuitive features. Our tool might only be useful for a few use cases, but we want it to be the best for those use cases.
