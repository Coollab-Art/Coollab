import React from "react";
import Link from "@docusaurus/Link";

type Props = {
  title: string;
  slug: string;
  author: string;
  image: string;
};

export default function DemoProjectCard({
  title,
  slug,
  author,
  image,
}: Props) {
  return (
    <Link
      to={slug}
      style={{
        textDecoration: "none",
        color: "inherit",
        border: "1px solid #e0e0e0",
        borderRadius: "12px",
        overflow: "hidden",
        display: "block",
      }}
    >
      <img
        src={image}
        alt={title}
        style={{
          width: "100%",
          height: "180px",
          objectFit: "cover",
          borderRadius: "12px 12px 0 0",
        }}
      />

      <div style={{ padding: "1rem" }}>
        <h3 style={{ margin: "0 0 0.3rem" }}>{title}</h3>
        <p style={{ margin: 0, fontSize: "0.85rem", color: "#888" }}>
          by {author}
        </p>
      </div>
    </Link>
  );
}