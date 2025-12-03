import React from "react"
import Layout from "@theme/Layout"
import BrowserOnly from "@docusaurus/BrowserOnly"

import loadable from "@loadable/component"
const Impl = loadable(() => import("../components/GalleryImpl"))

export default function () {
  return (
    <Layout title="Gallery" description="Artworks made by the community">
      <main>
        <BrowserOnly>{() => <Impl />}</BrowserOnly>
      </main>
    </Layout>
  )
}
