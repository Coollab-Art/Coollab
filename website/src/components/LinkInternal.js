import React from "react"
import Link from "@docusaurus/Link"

export default function ({ noDecoration, children, ...props }) {
  return (
    <Link style={noDecoration && { textDecoration: "none" }} {...props}>
      {children}
    </Link>
  )
}
