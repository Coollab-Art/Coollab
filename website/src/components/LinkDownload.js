import React from "react"

export default function ({ to, noDecoration, children, ...props }) {
  return (
    <a href={to} style={noDecoration && { textDecoration: "none" }} {...props}>
      {children}
    </a>
  )
}
