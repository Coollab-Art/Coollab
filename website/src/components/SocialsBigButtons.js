import React from "react"
import style from "./SocialsBigButtons.module.css"
import { FaInstagram, FaDiscord, FaMastodon } from "react-icons/fa"
import LinkExternal from "./LinkExternal"
import BigButton from "./BigButton"

export default function () {
  return (
    <div className={style.buttonsRow}>
      <LinkExternal to="https://www.instagram.com/coollab_art" noDecoration>
        <BigButton icon={<FaInstagram />}>Instagram</BigButton>
      </LinkExternal>
      <LinkExternal to="https://discord.com/invite/QEjqnEy4aT" noDecoration>
        <BigButton icon={<FaDiscord />}>Discord</BigButton>
      </LinkExternal>
      <LinkExternal to="https://mastodon.social/@Coollab" noDecoration>
        <BigButton icon={<FaMastodon />}>Mastodon</BigButton>
      </LinkExternal>
    </div>
  )
}
