const authors = {
  jules: {
    name: "Jules Fouchy",
    link: "https://julesfouchy.github.io/",
  },
}

const projects = [
  {
    title: "Carré Pattern",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/carre_pattern.png",
    download: "/demo-projects/carre_pattern.coollab",
  },
  {
    title: "Kaleidoscope",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/kaleidoscope.png",
    download: "/demo-projects/kaleidoscope.coollab",
  },
  {
    title: "Audioreactive Disk",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/audioreactive_disk.png",
    download: "/demo-projects/audioreactive_disk.coollab",
  },
  {
    title: "Chromatic Aberration",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/chromatic_aberration.png",
    download: "/demo-projects/chromatic_aberration.coollab",
  },
  {
    title: "Colored Squares",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/colored_squares.png",
    download: "/demo-projects/colored_squares.coollab",
  },
  {
    title: "Particules",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/particules.png",
    download: "/demo-projects/particules.coollab",
  },
  {
    title: "Soleils",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/soleils.png",
    download: "/demo-projects/soleils.coollab",
  },
  {
    title: "Space Noise",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/space_noise.png",
    download: "/demo-projects/space_noise.coollab",
  },
  {
    title: "Spiral Pattern",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/spiral_pattern.png",
    download: "/demo-projects/spiral_pattern.coollab",
  },
  {
    title: "Trailer Particles",
    authorId: "jules",
    image: "/img/demo-project-thumbnails/trailer_particles.png",
    download: "/demo-projects/trailer_particles.coollab",
  },
]

function slugify(title) {
  return title
    .toLowerCase()
    .normalize("NFD")
    .replace(/[\u0300-\u036f]/g, "") // remove accents
    .replace(/[^a-z0-9]+/g, "-") // non-alphanumeric to hyphens
    .replace(/^-|-$/g, "") // trim leading/trailing hyphens
}

module.exports = { authors, projects, slugify }
