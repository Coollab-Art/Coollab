const authors = {
  jules: {
    name: "Jules Fouchy",
    link: "https://julesfouchy.github.io/",
  },
}

const projects = [
  { title: "Carré Pattern", slug: "carre-pattern", authorId: "jules" },
  { title: "Kaleidoscope", slug: "kaleidoscope", authorId: "jules" },
  {
    title: "Audioreactive Disk",
    slug: "audioreactive-disk",
    authorId: "jules",
  },
  {
    title: "Chromatic Aberration",
    slug: "chromatic-aberration",
    authorId: "jules",
  },
  { title: "Colored Squares", slug: "colored-squares", authorId: "jules" },
  { title: "Particules", slug: "particules", authorId: "jules" },
  { title: "Soleils", slug: "soleils", authorId: "jules" },
  { title: "Space Noise", slug: "space-noise", authorId: "jules" },
  { title: "Spiral Pattern", slug: "spiral-pattern", authorId: "jules" },
  { title: "Trailer Particles", slug: "trailer-particles", authorId: "jules" },
]

function image(project) {
  return `/img/demo-project-thumbnails/${project.slug}.png`
}

function download(project) {
  return `/demo-projects/${project.slug}.coollab`
}

module.exports = { authors, projects, image, download }
