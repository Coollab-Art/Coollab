const fs = require("fs")
const path = require("path")
const { authors, projects } = require("../data/demo-projects")

const outputDir = path.resolve(
  __dirname,
  "../../content/Tutorials/02-Demo Projects/02-All Projects",
)
const thumbnailsDir = path.resolve(
  __dirname,
  "../../static/img/demo-project-thumbnails",
)
const imagesMapDir = path.resolve(
  __dirname,
  "../../.docusaurus/demo-projects-plugin",
)
const imagesMapFile = path.join(imagesMapDir, "images.json")

const STATIC_EXTS = ["png", "jpg", "jpeg", "webp", "avif"]
const ANIMATED_EXTS = ["gif"]
const THUMBNAILS_URL = "/img/demo-project-thumbnails"

function findFile(slug, exts) {
  for (const ext of exts) {
    const fileName = `${slug}.${ext}`
    if (fs.existsSync(path.join(thumbnailsDir, fileName))) {
      return fileName
    }
  }
  return null
}

function generateImagesMap() {
  const map = {}
  for (const project of projects) {
    map[project.slug] = {
      thumbnail: findFile(project.slug, STATIC_EXTS),
      image: findFile(project.slug, [...ANIMATED_EXTS, ...STATIC_EXTS]),
    }
  }
  fs.mkdirSync(imagesMapDir, { recursive: true })
  fs.writeFileSync(imagesMapFile, JSON.stringify(map, null, 2) + "\n")
  return map
}

function generate() {
  const imagesMap = generateImagesMap()

  // Clean previous generated files
  if (fs.existsSync(outputDir)) {
    for (const file of fs.readdirSync(outputDir)) {
      if (file.endsWith(".md")) {
        fs.unlinkSync(path.join(outputDir, file))
      }
    }
  }
  fs.mkdirSync(outputDir, { recursive: true })

  projects.forEach((project, index) => {
    const slug = project.slug
    const author = authors[project.authorId]
    const authorName = author ? author.name : "Unknown"

    const isFirst = index === 0
    const isLast = index === projects.length - 1

    const thumbFile = imagesMap[slug]?.thumbnail ?? `${slug}.png`
    const frontMatterImage = `${THUMBNAILS_URL}/${thumbFile}`

    const content = `---
title: ${project.title}
slug: /Demo Projects/${slug}
description: ${project.title} demo project shared by ${authorName}
keywords:
  - Coollab
  - Coollab demo
image: ${frontMatterImage}${isFirst ? "\npagination_prev: null" : ""}${isLast ? "\npagination_next: null" : ""}
---

import DemoProjectContent from '@site/src/components/DemoProjectContent'

<DemoProjectContent title="${project.title}" />
`

    const fileName = `${String(index).padStart(2, "0")}-${slug}.md`
    fs.writeFileSync(path.join(outputDir, fileName), content)
  })
}

module.exports = function demoProjectsPlugin() {
  generate()
  return {
    name: "demo-projects-plugin",
  }
}
