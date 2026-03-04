import React from "react";
import DemoProjectCard from "@site/src/components/DemoProjectCard/DemoProjectCard";
const { authors, projects, image } = require("@site/src/data/demo-projects");

export default function DemoProjectsGrid() {
  return (
    <div
      style={{
        display: "grid",
        gridTemplateColumns: "repeat(auto-fill, minmax(280px, 1fr))",
        gap: "1.5rem",
        marginTop: "2rem",
      }}
    >
      {projects.map((project) => {
        const author = authors[project.authorId];
        return (
          <DemoProjectCard
            key={project.slug}
            title={project.title}
            slug={`/Demo Projects/${project.slug}`}
            author={author ? author.name : "Unknown"}
            authorLink={author?.link}
            image={image(project)}
          />
        );
      })}
    </div>
  );
}
