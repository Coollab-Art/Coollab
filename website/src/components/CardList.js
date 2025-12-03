import React from "react"
import style from "./CardList.module.css"
import LinkExternal from "./LinkExternal"
import LinkInternal from "./LinkInternal"

// Only pass accentColor if you want to overwrite the one that will be set by CardList automatically
export function Card({
  icon,
  internalLink,
  externalLink,
  accentColor,
  compactCards,
  children,
}) {
  const cardContent = (
    <div className={style.card} style={{ borderColor: accentColor }}>
      <div className={style.cardIcon}>{icon}</div>
      <div
        className={`${style.cardText} ${
          !compactCards && style.notCompactCardText
        }`}
      >
        {/* Colorize all <b> tags */}
        {React.Children.map(children, (child) => {
          if (child.type === "b") {
            return React.cloneElement(child, {
              style: { ...child.props.style, color: accentColor },
            })
          }
          return child
        })}
      </div>
    </div>
  )

  return internalLink ? (
    <LinkInternal to={internalLink} style={{ textDecoration: "none" }}>
      {cardContent}
    </LinkInternal>
  ) : externalLink ? (
    <LinkExternal to={externalLink} style={{ textDecoration: "none" }}>
      {cardContent}
    </LinkExternal>
  ) : (
    <div>{cardContent}</div>
  )
}

export function CardList({ children, centered, compactCards }) {
  const accentColors = ["#F4AD7F", "#8656D7", "#30A7F5"]

  return (
    <div className={`${style.cardList} ${centered && style.centered}`}>
      {/* Add accentColor property to all children (which should be Card components) */}
      {React.Children.map(children, (child, i) => {
        const accentColor = child.props.accentColor || accentColors[i % 3]
        return React.cloneElement(child, { compactCards, accentColor })
      })}
    </div>
  )
}
