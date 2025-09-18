import React from "react"

export default function RandomizedList({ elements = [] }) {
  const shuffled = [...elements].sort(() => Math.random() - 0.5)

  return (
    <>
      {shuffled.map((el, i) =>
        React.isValidElement(el) ? (
          React.cloneElement(el, { key: i })
        ) : (
          <React.Fragment key={i}>{el}</React.Fragment>
        )
      )}
    </>
  )
}
