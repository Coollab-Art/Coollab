import React from "react"
import styles from "./GalleryImpl.module.css"
import LinkExternal from "./LinkExternal"
import LinkInternal from "./LinkInternal"

class GalleryImpl extends React.Component {
  constructor(props) {
    super(props)
    this.state = {
      images: [],
      Opened: false,
      currentImageIndex: null,
      previousImageIndex: [],
      nextImageIndex: [],
      index: 0,
      animate: false,
      transitionDirection: null,
    }
  }

  componentDidMount() {
    fetch("https://res.cloudinary.com/coollab/image/list/gallery.json")
      .then((response) => response.json())
      .then((data) => {
        const images = data.resources.map((info) => {
          const metadata = info.context !== undefined ? info.context.custom : {}
          return {
            url: `https://res.cloudinary.com/coollab/image/upload/v${info.version}/${info.public_id}.${info.format}`,
            title: metadata.title || "Untitled",
            description: metadata.description || "",
            author_name: metadata.author_name || "",
            author_link: metadata.author_link || "",
          }
        })
        this.setState({ images })
      })

    document.addEventListener("keydown", this.handleKeyDown)
    document.addEventListener("click", this.handleClick)
  }

  componentWillUnmount() {
    document.removeEventListener("keydown", this.handleKeyDown)
  }

  openImg = (index) => {
    this.setSurroundingsFullScreenImages(index)
    this.setState({
      Opened: true,
      transitionDirection: null,
      animate: false,
    })
  }

  setSurroundingsFullScreenImages = (index) => {
    this.setState({
      previousImageIndex:
        index - 1 < 0 ? this.state.images.length - 1 : index - 1,
      currentImageIndex: index,
      nextImageIndex: index + 1 > this.state.images.length - 1 ? 0 : index + 1,
    })
  }

  setPrevFullScreenImage = () => {
    this.setState({ transitionDirection: "left" }, () => {
      this.setSurroundingsFullScreenImages(this.state.previousImageIndex)
    })
  }

  setNextFullScreenImage = () => {
    this.setState({ transitionDirection: "right" }, () => {
      this.setSurroundingsFullScreenImages(this.state.nextImageIndex)
    })
  }

  getCurrentFullScreenImage = () => {
    return this.state.images[this.state.currentImageIndex].url
  }

  closeImg = () => {
    this.setState({ animate: true }, () => {
      setTimeout(() => {
        this.setState({ Opened: false, fullImageSrc: null, animate: false })
      }, 400)
    })
  }

  handleKeyDown = (event) => {
    if (this.state.Opened) {
      if (event.key === "Escape") this.closeImg()
      if (event.key === "ArrowLeft") this.setPrevFullScreenImage()
      if (event.key === "ArrowRight") this.setNextFullScreenImage()
    }
  }

  handleClick = (event) => {
    if (this.state.Opened) {
      const overlay = document.querySelector(`.${styles.imgOverlay}`)
      const isInsideOverlay = overlay && overlay.contains(event.target)

      const isButton =
        event.target.closest(`.${styles.prevButton}`) ||
        event.target.closest(`.${styles.nextButton}`) ||
        event.target.closest(`.${styles.closeButton}`)

      if (!isInsideOverlay || isButton) {
        return
      }

      this.closeImg()
    }
  }

  render() {
    const images = this.state.images.map((image, i) => (
      <div className={styles.galleryFrame} key={i}>
        <img
          src={image.url}
          className={styles.galleryImg}
          alt=""
          onClick={() => this.openImg(i)}
          style={{ animationDelay: `${i * 100}ms` }}
        />

        <div className={styles.galleryInfos}>
          <h2>{image.title}</h2>
          <h3>
            {image.author_name || image.author_link ? (
              <span>
                by{" "}
                {image.author_link ? (
                  <LinkExternal to={image.author_link}>
                    {image.author_name || "Unknown"}
                  </LinkExternal>
                ) : (
                  image.author_name || "Unknown"
                )}
              </span>
            ) : null}
          </h3>
          {image.description && <>{image.description}</>}
        </div>
      </div>
    ))

    return (
      <div className={styles.gallery}>
        <h2>Discover all of the community's incredible art!</h2>
        <h3>
          Submit your own artwork with{" "}
          <LinkInternal to="/Download">Coollab️</LinkInternal> 🎨
        </h3>

        <div
          className={styles.galleryImpl}
          style={{ pointerEvents: this.state.Opened ? "none" : "auto" }}
        >
          {images}
        </div>

        {this.state.Opened && (
          <div className={styles.imgOverlay}>
            <img
              key={`${this.state.Opened}-${this.state.currentImageIndex}-${
                this.state.transitionDirection || "open"
              }`}
              src={this.getCurrentFullScreenImage()}
              className={[
                styles.fullImage,
                this.state.animate
                  ? styles.animateScaleDown
                  : this.state.transitionDirection === "left"
                  ? styles.slideLeftEnter
                  : this.state.transitionDirection === "right"
                  ? styles.slideRightEnter
                  : styles.animateScaleUp,
              ].join(" ")}
              alt=""
            />
            <i
              className={`fa fa-times ${styles.closeButton}`}
              onClick={this.closeImg}
            ></i>
            <i
              className={`fa fa-arrow-left ${styles.prevButton}`}
              onClick={this.setPrevFullScreenImage}
            ></i>
            <i
              className={`fa fa-arrow-right ${styles.nextButton}`}
              onClick={this.setNextFullScreenImage}
            ></i>
          </div>
        )}
      </div>
    )
  }
}

export default GalleryImpl
