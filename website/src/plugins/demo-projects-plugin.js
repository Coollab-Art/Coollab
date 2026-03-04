const fs = require("fs")
const path = require("path")
const { authors, projects, image } = require("../data/demo-projects")

const outputDir = path.resolve(
  __dirname,
  "../../content/Tutorials/02-Demo Projects/02-All Projects",
)

function generate() {
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

    const content = `---
title: ${project.title}
slug: /Demo Projects/${slug}
description: ${project.title} demo project shared by ${authorName}
keywords:
  - Coollab
  - Coollab demo
image: ${image(project)}${isFirst ? "\npagination_prev: null" : ""}${isLast ? "\npagination_next: null" : ""}
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
