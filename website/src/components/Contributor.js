import React from "react"
import style from "./Contributor.module.css"
import LinkExternal from "./LinkExternal"

export default function ({ name, github_name, link, roles }) {
  return (
    <LinkExternal to={link}>
      <div className={style.contributorCard}>
        <img
          className={style.contributorAvatar}
          src={`https://avatars.githubusercontent.com/${github_name}`}
          alt={name}
        />
        <div>
          <div className={style.contributorName}>{name}</div>
          <div className={style.contributorRoles}>{roles.join(", ")}</div>
        </div>
      </div>
    </LinkExternal>
  )
}
