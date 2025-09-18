import React from "react"
import styles from "./Timeline.module.css"
import Image from "./Image"

export default function () {
  return (
    <Timeline>
      <Step img="/img/tuto1.png">
        Start with a<br />
        <b>Shape</b>
      </Step>
      <Step img="/img/tuto2.png">
        Add some
        <br />
        <b>Modifiers</b>
      </Step>
      <Step img="/img/tuto3.png">
        {/* <b>Colorize</b>! */}
        <b>Post-Process</b>
        <br />
        the result
      </Step>
    </Timeline>
  )
}
// TODO(Website) maybe make the timeline centered? And the line goes under the images, centered too, and each text is above the image
// TODO(Website) find a way for the line to automatically start and stop on first and last bullets ? instead of just hardcoding a size

function StepImage({ src, accentColor }) {
  return (
    <Image
      src={src}
      alt={src}
      className={styles.image}
      style={{ borderColor: accentColor }}
    />
  )
}

function Dot({ accentColor }) {
  return (
    <CenterInCell>
      <div className={styles.dot} style={{ backgroundColor: accentColor }} />
    </CenterInCell>
  )
}

function Number({ stepNumber, accentColor }) {
  return (
    <CenterInCell>
      <div className={styles.number} style={{ color: accentColor }}>
        {stepNumber}
      </div>
    </CenterInCell>
  )
}

function Text({ children, accentColor }) {
  return (
    <CenterInCell vertically>
      <div className={styles.text}>
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
    </CenterInCell>
  )
}

// Only pass accentColor if you want to overwrite the one that will be set by CardList automatically
function Step({ img, accentColor, stepNumber, children }) {
  return (
    <>
      <StepImage src={img} accentColor={accentColor} />
      <Dot accentColor={accentColor} />
      <Number stepNumber={stepNumber} accentColor={accentColor} />
      <Text accentColor={accentColor}>{children}</Text>
    </>
  )
}

function Timeline({ children }) {
  const accentColors = ["#F4AD7F", "#8656D7", "#30A7F5"]
  return (
    <>
      <div className={styles.timeline}>
        {React.Children.map(children, (child, i) => {
          const accentColor = child.props.accentColor || accentColors[i % 3]
          return React.cloneElement(child, {
            accentColor,
            stepNumber: i + 1,
            isFirstStep: i === 0,
            isLastStep: i === children.length - 1,
          })
        })}
      </div>
    </>
  )
}

function CenterInCell({ children, vertically }) {
  return (
    <div
      className={
        vertically ? styles.centerVerticallyInCell : styles.centerInCell
      }
    >
      {children}
    </div>
  )
}
