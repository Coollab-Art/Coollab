import React from "react"
import Layout from "@theme-original/Layout"
import NavbarScrollEffect from "../../components/NavbarScrollEffect"

export default function LayoutWrapper(props) {
  return (
    <>
      <Layout {...props} />
      <NavbarScrollEffect />
    </>
  )
}
