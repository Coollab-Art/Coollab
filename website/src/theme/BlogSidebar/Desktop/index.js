import React, { memo } from "react";
import clsx from "clsx";
import { translate } from "@docusaurus/Translate";
import {
  useVisibleBlogSidebarItems,
  BlogSidebarItemList,
} from "@docusaurus/plugin-content-blog/client";
import BlogSidebarContent from "@theme/BlogSidebar/Content";
import styles from "./styles.module.css";

const ListComponent = ({ items }) => {
  return (
    <BlogSidebarItemList
      items={items}
      ulClassName={clsx(styles.sidebarItemList, "clean-list")}
      liClassName={styles.sidebarItem}
      linkClassName={styles.sidebarItemLink}
      linkActiveClassName={styles.sidebarItemLinkActive}
    />
  );
};

function BlogSidebarDesktop({ sidebar }) {
  const items = useVisibleBlogSidebarItems(sidebar.items);
  return (
    <aside className={styles.sidebarContainer}>
      <div className={styles.sidebarViewport}>
        <nav
          className={clsx(styles.sidebar, "thin-scrollbar")}
          aria-label={translate({
            id: "theme.blog.sidebar.navAriaLabel",
            message: "Blog recent posts navigation",
            description:
              "The ARIA label for recent posts in the blog sidebar",
          })}
        >
          <BlogSidebarContent
            items={items}
            ListComponent={ListComponent}
            yearGroupHeadingClassName={styles.yearGroupHeading}
          />
        </nav>
      </div>
    </aside>
  );
}

export default memo(BlogSidebarDesktop);
