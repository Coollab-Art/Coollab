import React from "react"
import style from "./GradientDownloadButton.module.css"
import LinkInternal from "./LinkInternal"

export default function () {
  return (
    <LinkInternal className={style.gradientDownloadButton} to="/Download">
      Download
    </LinkInternal>
  )
}
