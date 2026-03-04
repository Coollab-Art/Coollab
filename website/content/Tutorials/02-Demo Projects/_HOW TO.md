# How to add a new demo project

All demo project pages are auto-generated from the data file at `src/data/demo-projects.js`.

## Adding a new project

1. Add the `.coollab` file to `static/demo-projects/`
2. Add the thumbnail image to `static/img/demo-project-thumbnails/`
3. Add an entry to the `projects` array in `src/data/demo-projects.js`:

That's it! The page and grid card are generated automatically.

## Adding a new author

Add an entry to the `authors` object in `src/data/demo-projects.js`:

```js
alice: {
  name: "Alice",
  link: "https://example.com/alice", // optional, can be `undefined`
},
```

Then use `authorId: "alice"` in the project entry.

# Customizing the page template

Go to `demo-projects-plugin.js`