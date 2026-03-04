import React from "react"
import style from "./BigButton.module.css"

export default function ({ icon, children, noCapsLock, fitContent }) {
  return (
    <div
      className={`${style.button} ${!noCapsLock && style.capsLock}`}
      style={fitContent ? { width: "fit-content" } : undefined}
    >
      {React.cloneElement(icon, { className: style.icon })} {children}
    </div>
  )
}
