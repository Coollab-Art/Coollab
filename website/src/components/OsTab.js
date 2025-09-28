import React, { useEffect, useState } from "react"
import Tabs from "@theme/Tabs"
import TabItem from "@theme/TabItem"

export default function OsTab({ children }) {
  const [defaultTab, setDefaultTab] = useState("windows") // Default to Windows

  useEffect(() => {
    const userAgent = navigator.userAgent
    if (userAgent.includes("Mac")) {
      setDefaultTab("mac")
    } else if (userAgent.includes("Linux")) {
      setDefaultTab("linux")
    } else {
      setDefaultTab("windows") // Fallback to Windows
    }
  }, [])

  const osTabs = {
    windows: null,
    linux: null,
    mac: null,
  }

  // Ensure children are properly assigned
  React.Children.forEach(children, (child) => {
    if (React.isValidElement(child)) {
      osTabs[child.props.value] = child
    }
  })

  return (
    <Tabs groupId="os-tabs" defaultValue={defaultTab}>
      {osTabs.windows}
      {osTabs.linux}
      {osTabs.mac}
    </Tabs>
  )
}
