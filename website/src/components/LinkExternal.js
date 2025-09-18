import React from "react"

export default function ({ to, noDecoration, children, ...props }) {
  return (
    <a
      href={to}
      target="_blank"
      rel="noopener noreferrer"
      style={noDecoration && { textDecoration: "none" }}
      {...props}
    >
      {children}
    </a>
  )
}
