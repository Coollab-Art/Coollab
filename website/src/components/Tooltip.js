import React, { useState } from "react"
import style from "./Tooltip.module.css"

export default ({ children, text }) => {
  const [visible, setVisible] = useState(false)

  return (
    <div
      className={style.tooltip}
      onMouseEnter={() => setVisible(true)}
      onMouseLeave={() => setVisible(false)}
    >
      {children}
      <span
        className={`${style.tooltipText} ${
          visible ? style.tooltipTextVisible : ""
        }`}
      >
        {text}
      </span>
    </div>
  )
}
