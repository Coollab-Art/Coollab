import React from "react"
import styles from "./FFmpegLegalNotice.module.css"
import LinkExternal from "./LinkExternal"

export default function FFmpegLegalNotice() {
  return (
    <p className={styles.ffmpegLegalNotice}>
      This software uses code of{" "}
      <LinkExternal to="https://ffmpeg.org/">FFmpeg</LinkExternal> licensed
      under the{" "}
      <LinkExternal to="https://www.gnu.org/licenses/gpl-3.0.html">
        GPLv3.0
      </LinkExternal>{" "}
      and its source can be downloaded{" "}
      <LinkExternal to="https://github.com/Coollab-Art/Coollab">
        here
      </LinkExternal>
    </p>
  )
}
