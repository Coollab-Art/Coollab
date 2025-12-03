
## Important tools

If you are only making a single one-off small contribution you don't necessarily need them, but otherwise please install:

- [clang-format](https://julesfouchy.github.io/Learn--Clean-Code-With-Cpp/lessons/formatting-tool/) to automatically format your code in a way that is consistent with the rest of the codebase

- [clang-tidy](https://julesfouchy.github.io/Learn--Clean-Code-With-Cpp/lessons/static-analysers/) which can detect some programming mistakes, and warn you if you don't respect our coding style (it doesn't find everything, but it is a good first pass)

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