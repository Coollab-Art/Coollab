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
