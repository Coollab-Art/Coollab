import React from "react"
import style from "./ArgumentsCards.module.css"
import Image from "./Image"

export default function ArgumentsCards() {
  return (
    // TODO(Website) make components so that it reads better instead of having a wall of div
    // TODO(Website) the images should probably just be 3 nice artworks made with coollab. Or for "100s of effects" the picture with many small squares of effects / the nodes menu (the future one with all the thumbnails)
    // TODO(Website) maybe make these cards a bit bigger?
    <div className={style.cardContainer}>
      <div className={style.card} style={{ borderColor: "#8656D7" }}>
        <div className={style.cardImage}>
          <Image src="/img/demo.png" noSmall></Image>
        </div>
        <p className={style.cardText}>
          DESIGNED TO BE <span style={{ color: "#8656D7" }}>SIMPLE</span> AND{" "}
          <span style={{ color: "#8656D7" }}>INTUITIVE</span>
        </p>
      </div>
      <div className={style.card} style={{ borderColor: "#F4AD7F" }}>
        <div className={style.cardImage}>
          <Image src="/img/vj.jpg" noSmall></Image>
        </div>
        <p className={style.cardText}>
          <span style={{ color: "#F4AD7F" }}>FREE</span> AND
          <br /> <span style={{ color: "#F4AD7F" }}>OPEN-SOURCE</span>
          {/*  FOREVER */}
        </p>
      </div>
      <div className={style.card} style={{ borderColor: "#30A7F5" }}>
        <div className={style.cardImage}>
          <Image src="/img/demo-horizontal.png" noSmall></Image>
        </div>
        <p className={style.cardText}>
          {/* A powerful tool for <span style={{ color: "#30A7F5" }}>Vjing</span>{" "}
          and so much more! */}
          <span style={{ color: "#30A7F5" }}>100s</span> OF EFFECTS
          <br /> <span style={{ color: "#30A7F5" }}>∞</span> COMBINATIONS
        </p>
      </div>
    </div>
  )
}
