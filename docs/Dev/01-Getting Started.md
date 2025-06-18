## Clone

⚠ Since we use Git submodules you need to clone them alongside the project. The easiest way is to use the command:
```
git clone --recursive https://github.com/Coollab-Art/Coollab
```

If while compiling you get an error like:
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

## Build

Coollab is built using CMake. If you don't know how to install and use it, [here is a tutorial](https://github.com/JulesFouchy/Simple-Cpp-Setup?tab=readme-ov-file#simple-cpp-setup).

If you want some help with C++ tooling (compiler, IDE extensions etc.) : https://julesfouchy.github.io/Learn--Clean-Code-With-Cpp/lessons/install-a-compiler/

I would recommend using the Cmake VS Code extension : cf this readme https://github.com/JulesFouchy/Simple-Cpp-Setup?tab=readme-ov-file#simple-cpp-setup



## Important tools

Ideally, please install [clang-format](https://julesfouchy.github.io/Learn--Clean-Code-With-Cpp/lessons/formatting-tool/) and [clang-tidy](https://julesfouchy.github.io/Learn--Clean-Code-With-Cpp/lessons/static-analysers/). Clang-format will format your code automatically, and clang-tidy will warn you when you make programming mistakes. These are two very important tools!

## Working with submodules

We use Git submodules: this means 

We mostly have two repositories:
- **Coollab**: the actual 
- **Cool**: a framework, with all our UI code, generic things etc (This framework is also used by the [launcher](TODO))

I would recommend using [Git Fork](TDOO) as a git UI, it knows how to work with submodules (unlike Git Kraken which is very very bad at it and will cause you many problems)

- Whenever you change branch on Lab, go to Cool and make sure to exit the "Detached HEAD" mode.
- When creating a branch: create one branch on Lab and one on Cool, with the same names
- When merging main into your branch: Lab first, then Cool and commit, then go back to Lab and commit only once Cool is at the right version
- When merging PR: merge Cool first, then go to Lab and update it so that it points to the main branch of Cool, then you can merge the Lab PR

## Navigating the code

CTRL+SHIFT+I in imgui

## Project management

We create big issues with the `Type` Big Plan. The first comment you should edit wheenver you need to add a point to do, and then the discussion below is here to discuss, propose ideas, and once they are validated we add it as a checkboxx todo in the main message 

### When starting to work on something

Fork for devs not part of the org

**Never commit on the *main* branch!**
We always work on branches, and then create pull requests that must be reviewed by others before merging them into main.

- Create a branch on *Cool*, and a branch on *Lab*. **They should both have the same name**.
- Switching between your branch and main is annoying because we use submodules and you would have two branches to switch and it can be painful. This is why we **strongly** recommend that you clone the repository twice, and you always leave one of them on main, and the other on your branch.

### Dev Tips

- TODO create a page explaining how to edit the Default User Data (imgui.ini, color_config.json, etc)
- If you add a third-party library, check its license and, if it requires it, make sure we install the license in the CMake:
```cmake
install(FILES "lib/cpp-httplib/LICENSE" DESTINATION license/cpp-httplib)
```

- TODO Talk about debug options (and you can create your own)

- TODO Talk about shortcut for ImGui Item Picker
- TODO Talk about the ImGui demo window

- TODO how to run the tests
- TODO how to write tests

### Fix bugs and bad code as you see them

Or at least write them down [in our To-Do list](https://github.com/Coollab-Art/Coollab/issues/127).<br/>
No codebase is perfect and bugs and poor code exist everywhere. If you think you just spotted one, take a deeper look into it. Try to fix it or at least raise other people's attention about it.

### Gitmoji

You can see the ones we use regularly in the [tooling](https://github.com/Coollab-Art/tooling/blob/main/gitmoji.md) folder (which is embeded in all our repos).

### Git

- Try to avoid mixing _unrelated_ changes in a single commit: If you have made lots of changes, consider pushing them in separate commits that make sense.

### Write nice Pull Requests

- merge `main` into your branch and resolve any conflict that might arise
- submit a pull request and wait for the peer review
- once the review is done the reviewer will merge your pull request

Before you submit your PR for review:

- Double check the names you gave. Ask yourself: would someone who doesn't know the codebase have a clue as to what `functionXxx()` does?
- Look for duplicated code and refactor it away.
- Make sure the code will be easy to change in the future.

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
