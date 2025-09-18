import React from "react"
import styles from "./GalleryPreview.module.css"
import Spacer from "./Spacer"

// TODO(Website) We are only showcasing 3D images, which is not Coollab's main goal. Need to change that
const images = [
  "/img/lab/1.png",
  "/img/lab/3.png",
  "/img/lab/5.png",
  "/img/lab/10.png",
  "/img/lab/7.png",
  "/img/lab/9.png",
]

export default function GalleryPreview() {
  const rows = []
  for (let i = 0; i < images.length; i += 2) {
    rows.push(images.slice(i, i + 2))
  }

  return (
    <section className={styles.gallerySection}>
      {rows.map((pair, rowIndex) => {
        const isEven = rowIndex % 2 === 0
        const firstImage = isEven ? pair[0] : pair[1]
        const secondImage = isEven ? pair[1] : pair[0]
        const isLastRow = rowIndex === rows.length - 1

        return (
          <>
            <div key={rowIndex} className={styles.row}>
              <div
                className={`${styles.imageTile} ${
                  isEven ? styles.narrow : styles.wide
                }`}
                style={{ backgroundImage: `url(${firstImage})` }}
              />
              <div
                className={`${styles.imageTile} ${
                  isEven ? styles.wide : styles.narrow
                }`}
                style={{ backgroundImage: `url(${secondImage})` }}
              />
            </div>
            {!isLastRow && <Spacer height="2rem" />}
          </>
        )
      })}
    </section>
  )
}
