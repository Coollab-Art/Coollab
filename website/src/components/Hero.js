import React from "react"
import style from "./Hero.module.css"

export default function () {
  return (
    <main className={style.heroSection}>
      <div className={style.heroBackground}></div>
      <div className={style.heroContent2}>
        <div className={style.heroContent}>
          <h1 className={style.heroTitle}>COOLLAB</h1>
          <p className={style.heroSubtitle}>
            Node-based creation for everyone
            <br />
            VJs, Coders, Artists, and the like
          </p>
        </div>
      </div>
      <div className={style.svgWave}>
        <svg
          width="100%"
          height="auto"
          viewBox="0 0 1440 265"
          xmlns="http://www.w3.org/2000/svg"
          preserveAspectRatio="none"
        >
          <path
            d="M 928.556,199.183 C 1132.9505,249.503 1389.58,122.585 1448,0.314806 L 1443.4211,283.63395 -37.545886,284.77868 -37.3233,237.364 c -66.1371,36.76812 65.460273,-73.09441 153.24722,-41.17628 87.78694,31.91812 227.45095,105.32399 369.90508,36.58528 142.45413,-68.73871 238.3325,-83.91 442.727,-33.59 z"
            fill="#27272A"
          />
        </svg>
      </div>
    </main>
  )
}
