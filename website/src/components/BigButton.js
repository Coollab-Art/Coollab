import React from "react"
import style from "./BigButton.module.css"

export default function ({ icon, children }) {
  return (
    <div className={style.button}>
      {React.cloneElement(icon, { className: style.icon })} {children}
    </div>
  )
}
