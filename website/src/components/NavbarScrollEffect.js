import { useEffect } from "react"

export default function NavbarScrollEffect() {
  useEffect(() => {
    const navbar = document.querySelector(".navbar")

    const handleScroll = () => {
      if (!navbar) return
      if (window.scrollY > 10) {
        navbar.classList.add("navbar--scrolled")
      } else {
        navbar.classList.remove("navbar--scrolled")
      }
    }

    window.addEventListener("scroll", handleScroll)
    return () => window.removeEventListener("scroll", handleScroll)
  }, [])

  return null
}
