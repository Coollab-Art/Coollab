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

## Supported compilers

- On Windows: **Clang** and **MSVC**
- On Linux: **Clang** and **GCC**
- On MacOS: **Clang**

## Build

Coollab is built using CMake. If you don't know how to compile a C++ project, here is a complete tutorial:
- [Follow this section to install all the tools](https://github.com/JulesFouchy/Simple-Cpp-Setup?tab=readme-ov-file#installing-the-tools) (compiler, IDE, CMake, etc.)
- [Then this one to compile and run the project](https://github.com/JulesFouchy/Simple-Cpp-Setup?tab=readme-ov-file#running-the-code)

## Navigating the code

Now that you can run Coollab, one of the best ways to find the part of the code that you want to modify is to use the **ImGui Item Picker**.

ImGui is the library that we use to create all of our UI, and it has a very powerful tool that allows you to click on any widget inside the app and it will break your debugger at the corresponding place in the code.

So for example if you wanted to find where the Video Export happens, you would:
- Start Coollab (**with a debugger attached** (i.e. using the 🐞 icon in VS Code))
![](https://github.com/user-attachments/assets/2a64799d-5810-4d14-bf5f-8c54e981b9d4)
- Go to the Video Export window (or whichever other part of the UI does the thing that you are interested in)
- Press <kbd>CTRL</kbd>+<kbd>SHIFT</kbd>+<kbd>I</kbd>, and then click on the export button
- It will break somewhere in the code: You will need to go up in the callstack a bit, as the breakpoint will trigger inside some internal ImGui functions, and then you should find the code that does the logic corresponding to the UI widget you just clicked on

https://github.com/user-attachments/assets/b0b2f9d0-db30-421d-a9ba-ab61034623c7


TODO link to next section

[Once you are done with your changes, read on to the next section to learn how to submit them](./02-Submitting%20your%20first%20code%20changes.md)