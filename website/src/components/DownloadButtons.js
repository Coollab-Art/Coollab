import React from "react"
import style from "./DownloadButtons.module.css"
import { FaWindows, FaApple, FaLinux } from "react-icons/fa"
import FFmpegLegalNotice from "./FFmpegLegalNotice"
import CodeBlock from "@theme/CodeBlock"
import BigButton from "./BigButton"
import LinkExternal from "./LinkExternal"
import LinkDownload from "./LinkDownload"
import Tooltip from "./Tooltip"

export default function () {
  return (
    <>
      <div className={style.buttonsRow}>
        {/* Windows */}
        <LinkDownload to="/Coollab-Launcher-Windows.exe" noDecoration>
          <BigButton icon={<FaWindows />}>Windows</BigButton>
        </LinkDownload>
        {/* Linux */}
        <Tooltip
          text={
            <>
              If you need help running the AppImage,
              <br />
              check out{" "}
              <LinkExternal to="https://appimage.org/">
                the official AppImage website
              </LinkExternal>
              <br />
              <br />
              Also, you might need to
              <CodeBlock inline>sudo apt install libegl1-mesa-dev</CodeBlock>
            </>
          }
        >
          <LinkDownload to="/Coollab-Launcher.AppImage" noDecoration>
            <BigButton icon={<FaLinux />}>Linux</BigButton>
          </LinkDownload>
        </Tooltip>
        {/* MacOS */}
        <LinkDownload to="/Coollab-Launcher-MacOS.zip" noDecoration>
          <BigButton icon={<FaApple />}>MacOS</BigButton>
        </LinkDownload>
      </div>{" "}
      <FFmpegLegalNotice />
    </>
  )
}
