# Coollab

<img alt="logo" src="./logo.png" width = 100px>

**Coollab is a node-based software to create visual effects and generative-art, similar to TouchDesigner.**

[Check out our website](https://coollab-art.com/)!

Coollab's main goal is to be as intuitive and easy to use as possible. Effects are high-level and they just work out of the box! You don't need to know any math or programming to make your own very cool effects! (Although having those skills allows you to do some advanced stuff).<br/>
Start with any image or shape, like your *Webcam*, add any modifier on top of it, like a *Kaleidoscope*, and you already have some cool results. You can then play with the 100s of effects to change the color, distort the image, add particles, react to audio, and much more!<br/>
Coollab can be used for VJing, exporting videos, creating interactive installations, *etc.*!

[Here are some tutorials to help you get started.](https://youtu.be/swMZGsGje4E?list=PLuMMMaL5kN3mG6fSeSYTqC-sc_OTX3unQ)

## Dev

### Clone

⚠ Since we use Git submodules you need to clone them alongside the project. The easiest way is to use the command:
```
git clone --recursive https://github.com/Coollab-Art/Coollab
```

If later on while compiling you get an error like:
```
[cmake] CMake Error at CMakeLists.txt:44 (add_subdirectory):
[cmake]   The source directory
[cmake] 
[cmake]     C:/Users/fouch/Downloads/Coollab/Cool
[cmake] 
[cmake]   does not contain a CMakeLists.txt file.
```

it most likely means you forgot the submodules. To grab them after having cloned the project, use:
```
git submodule update --init --recursive
```

### Build

Coollab is built using CMake. If you don't know how to install and use it, [here is a tutorial](https://github.com/JulesFouchy/Simple-Cpp-Setup?tab=readme-ov-file#simple-cpp-setup).

### Important tools

Install [clang-format](https://julesfouchy.github.io/Learn--Clean-Code-With-Cpp/lessons/formatting-tool/) and [clang-tidy](https://julesfouchy.github.io/Learn--Clean-Code-With-Cpp/lessons/static-analysers/). Clang-format will format your code automatically, and clang-tidy will warn you when you make programming mistakes. These are two very important tools!

### Dev Tips

- TODO create a page explaining how to edit the Default User Data (imgui.ini, color_config.json, etc)
- If you add a third-party library, check its license and, if it requires it, make sure we install the license in the CMake:
```cmake
# LICENSE
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}\\\\LICENSE")
install(FILES "lib/cpp-httplib/LICENSE" DESTINATION license/cpp-httplib)
```

- TODO Talk about debug options (and you can create your own)

- TODO Talk about shortcut for ImGui Item Picker
- TODO Talk about the ImGui demo window

- TODO how to run the tests

### Branches

:::tip
**Never commit on the *main* branch!**<br/>
We always work on branches, and then create pull requests that must be reviewed by others before merging them into main.
:::

- Create a branch on *Cool*, and a branch on *CoolLab*. **They should both have the same name**.
- Switching between your branch and main is annoying because we use submodules and you would have two branches to switch and it can be painful. This is why we **strongly** recommend that you clone the repository twice, and you always leave one of them on main, and the other on your branch.

### Creating a new library

If you are about to create a small system that doesn't have any dependencies on the rest of *Cool*, then making it into a standalone library makes a lot of sense.<br/>
We try to do that whenever possible because it encourages a more decoupled and flexible design, and makes it easily reusable by others without having to include the whole of *Cool*.

- Create a new repository from [our template](https://github.com/Coollab-Art/library-template):
    - ⚠️ **Make sure the _Owner_ is set to Coollab-Art, not to your personal account!**
![set_owner_to_coollab_art](https://github.com/user-attachments/assets/e2fc0d5f-f2ee-4867-ba29-15c05ddbdb4e)
    - Add a *short* description. (This is optional, you can add it later if you want)
    - Make sure the repo is public
    - Don't check `Include all branches`
    - Click `Create repository from template`
- Add the library to [our list of **Standalone libraries**](https://github.com/Coollab-Art/.github/edit/main/profile/README.md) under `## Standalone libraries` by adding:
```markdown
<a href="https://github.com/Coollab-Art/mynewlib">
  <img align="center" src="https://github-readme-stats.vercel.app/api/pin/?username=Coollab-Art&repo=mynewlib" />
</a>
```
(NB: Replace the two occurences of `mynewlib` with the name of your library)
- Clone the repo **recursively**: `git clone --recursive url/to/the/new/repo`
- In the cloned repo, fill in "setup.py" and run the script: it will setup everything and then remove the Python scripts used for the setup and amend the initial commit
- Commit as "🎉 Initial commit" and `force push` it
- Implement the library
