import React, { useState, useEffect } from "react"

function appendSmall(path) {
  const dotIndex = path.lastIndexOf(".")
  if (dotIndex === -1) return path + " small" // no extension
  return path.slice(0, dotIndex) + " small" + path.slice(dotIndex)
}

function preloadImage(src) {
  const img = new Image()
  img.loading = "lazy"
  img.src = src
}

export default function ({
  src,
  alt,
  style,
  noSmall /* TODO remove this noSmall, images should always have small preview*/,
  ...props
}) {
  const [isOpen, setIsOpen] = useState(false)
  const [visible, setVisible] = useState(false)

  const smallImageUrl = noSmall ? src : appendSmall(src)
  const bigImageUrl = src

  preloadImage(bigImageUrl)

  const open = () => setIsOpen(true)

  const close = () => {
    setVisible(false)
    setTimeout(() => {
      setIsOpen(false)
    }, 300)
  }

  useEffect(() => {
    if (isOpen) {
      requestAnimationFrame(() => setVisible(true))
    }
  }, [isOpen])

  return (
    <>
      <img
        src={smallImageUrl}
        alt={alt}
        style={{ cursor: "pointer", ...style }}
        onClick={open}
        {...props}
      />

      {isOpen && (
        <div
          onClick={close}
          style={{
            position: "fixed",
            top: 0,
            left: 0,
            width: "100vw",
            height: "100vh",
            backgroundColor: "rgba(0,0,0,0.8)",
            display: "flex",
            justifyContent: "center",
            alignItems: "center",
            zIndex: 9999,
            cursor: "pointer",
            opacity: visible ? 1 : 0,
            transition: "opacity 0.3s ease",
          }}
        >
          <img
            src={bigImageUrl}
            alt={alt}
            style={{
              maxWidth: "90%",
              maxHeight: "90%",
              boxShadow: "0 0 20px rgba(0,0,0,0.5)",
              transform: visible ? "scale(1)" : "scale(0.95)",
              transition: "transform 0.3s ease, opacity 0.3s ease",
            }}
          />
        </div>
      )}
    </>
  )
}
