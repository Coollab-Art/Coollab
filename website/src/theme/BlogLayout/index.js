import React from "react";
import Layout from "@theme/Layout";
import BlogSidebar from "@theme/BlogSidebar";
import styles from "./styles.module.css";

export default function BlogLayout(props) {
  const { sidebar, toc, children, ...layoutProps } = props;
  const hasSidebar = sidebar && sidebar.items.length > 0;

  return (
    <Layout
      {...layoutProps}
      wrapperClassName={styles.blogWrapper}
    >
      <div className={styles.blogPage}>
        {hasSidebar && (
          <BlogSidebar sidebar={sidebar} />
        )}
        <main className="container margin-vert--lg">
          {children}
        </main>
      </div>
    </Layout>
  );
}
