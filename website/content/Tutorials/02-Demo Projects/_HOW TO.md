# How to add a new demo project

All demo project pages are auto-generated from the data file at `src/data/demo-projects.js`.

## Adding a new project

1. Add an entry to the `projects` array in `src/data/demo-projects.js` with `title`, `slug`, and `authorId`
2. Add the `.coollab` file to `static/demo-projects/<slug>.coollab`
3. Add the thumbnail image to `static/img/demo-project-thumbnails/<slug>.<ext>` (`.png`, `.jpg`, `.jpeg`, `.webp` or `.avif`). The thumbnail is used for the card on the all-projects page and as the page's social preview.
4. Optionally also add `static/img/demo-project-thumbnails/<slug>.gif` — when present, the project page itself will display the gif instead of the static thumbnail.

The `slug` is used to derive both the URL (`/Demo Projects/<slug>`) and the file paths automatically.

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