import React from "react";
import Link from "@docusaurus/Link";
import {useDoc} from '@docusaurus/plugin-content-docs/client';
import DownloadProjectButton from "@site/src/components/DownloadProjectButton";
const { authors, projects, image, download } = require("@site/src/data/demo-projects");

type Props = {
  title: string;
};

function MiniPaginator() {
  const {metadata} = useDoc();
  const {previous, next} = metadata;
  if (!previous && !next) return null;
  return (
    <nav
      style={{
        display: "flex",
        justifyContent: "space-between",
        alignItems: "center",
        marginBottom: "1.5rem",
      }}
    >
      {previous ? (
        <Link
          to={previous.permalink}
          style={{
            display: "inline-flex",
            alignItems: "center",
            gap: "0.4rem",
            fontSize: "0.9rem",
            color: "#30a7f5",
            textDecoration: "none",
          }}
        >
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
            <polyline points="15 18 9 12 15 6" />
          </svg>
          {previous.title}
        </Link>
      ) : <span />}
      {next ? (
        <Link
          to={next.permalink}
          style={{
            display: "inline-flex",
            alignItems: "center",
            gap: "0.4rem",
            fontSize: "0.9rem",
            color: "#30a7f5",
            textDecoration: "none",
          }}
        >
          {next.title}
          <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round">
            <polyline points="9 18 15 12 9 6" />
          </svg>
        </Link>
      ) : <span />}
    </nav>
  );
}

export default function DemoProjectContent({ title }: Props) {
  const project = projects.find((p) => p.title === title);
  if (!project) {
    return <p>Project not found: {title}</p>;
  }

  const author = authors[project.authorId];
  const authorName = author ? author.name : "Unknown";
  const authorLink = author?.link;

  return (
    <>
      <MiniPaginator />

      <div style={{ marginBottom: "1.5rem" }}>
        <img
          src={image(project)}
          alt={project.title}
          style={{
            width: "100%",
            borderRadius: "var(--radius-mid, 10px)",
          }}
        />
      </div>

      <div style={{ display: "flex", justifyContent: "center" }}>
        <DownloadProjectButton link={download(project)} />
      </div>

      <div
        style={{
          display: "flex",
          alignItems: "center",
          justifyContent: "center",
          gap: "0.8rem",
          marginTop: "1.5rem",
        }}
      >
        <div
          style={{
            display: "inline-flex",
            alignItems: "center",
            gap: "0.5rem",
            background: "var(--coollab-secondary-color)",
            borderRadius: "999px",
            padding: "0.3rem 0.9rem",
            fontSize: "0.9rem",
            color: "white",
          }}
        >
          Shared by{" "}
          {authorLink ? (
            <a
              href={authorLink}
              target="_blank"
              rel="noopener noreferrer"
              style={{
                color: "white",
                fontWeight: "bold",
              }}
            >
              {authorName}
            </a>
          ) : (
            <strong style={{ color: "white" }}>
              {authorName}
            </strong>
          )}
        </div>
        <Link
          to="/Contribute/Outreach#share-what-you-do-with-coollab"
          style={{
            fontSize: "0.85rem",
            color: "#30a7f5",
          }}
        >
          Share your own projects!
        </Link>
      </div>
    </>
  );
}
