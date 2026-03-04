import React from "react"
import { FaFileDownload } from "react-icons/fa"
import BigButton from "./BigButton"
import LinkDownload from "./LinkDownload"

export default function ({ link }) {
  return (
    <LinkDownload to={link} noDecoration>
      <BigButton icon={<FaFileDownload />} noCapsLock>
        Download project file
      </BigButton>
    </LinkDownload>
  )
}
