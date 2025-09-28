- If new windows have been added, make sure that *imgui.ini* has been updated in *User data Default*
- Choose a name for the version and add it in *changelog.md*, *CMakeLists.txt* and *Info.plist* (in CFBundleVersion AND CFBundleShortVersionString)
- At the top of *versions_compatibility.txt*, add the name of the version, and incompatibility markers if necessary
- Add gifs and images in the changelog to illustrate the changes
- Commit with the message "🔖 [version_name] Release" (make sure to replace with the right version name)
- On GitHub, create a new release with the name of the version you just chose. Use a new tag with the same name as the version. As a description, use whatever is in the changelog for the new version
- Make an announcement on our Discord + thank the contributors if any (ptut, interns, etc.)
- Make a story on our Instagram
- Make a toot on Mastodon

## Choosing a version name

Our names look like "1.0.0 Launcher". There is a friendly name that should give users a good idea of the main feature added in this version (e.g. "Launcher", "Particles", "WebGPU", etc.). If the release only contains bug fixes / minor changes that don't need to be highlighted, then keep the same friendly name as the previous version, and just increment the patch number. If you changed the friendly name, then increase the minor number instead (or the major one if this is something really big that changes the way Coollab looks and feels).
Our version numbers **DO NOT** reflect the (in)compatibilities between versions. We have *versions_compatibility.txt* for that. Instead the goal is to help users easily identify each version, and what changes between them.