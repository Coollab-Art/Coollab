import React, { useState, useEffect } from "react"
import style from "./SectionTitle.module.css"

export default function ({ fontSize, mobileFontSize, children }) {
  const [size, setSize] = useState(fontSize)

  mobileFontSize = mobileFontSize || fontSize

  useEffect(() => {
    const handleResize = () => {
      setSize(window.innerWidth <= 768 ? mobileFontSize : fontSize)
    }

    handleResize() // set initial size
    window.addEventListener("resize", handleResize)
    return () => window.removeEventListener("resize", handleResize)
  }, [fontSize, mobileFontSize])

  return (
    <h2 className={style.sectionTitle} style={{ fontSize: size }}>
      {children}
    </h2>
  )
}
