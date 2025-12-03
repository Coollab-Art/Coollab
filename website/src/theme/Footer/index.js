import React from "react"
import styles from "./styles.module.css"
import { FaDiscord, FaInstagram, FaMastodon, FaGithub } from "react-icons/fa"
import LinkExternal from "../../components/LinkExternal"
import LinkInternal from "../../components/LinkInternal"

export default function Footer() {
  const year = new Date().getFullYear()

  return (
    <footer className={styles.footer}>
      <div className={styles.container}>
        {}
        <div className={styles.top}>
          {}
          <div className={styles.brand}>
            <img
              src="/img/logo.png"
              alt="Coollab Logo"
              className={styles.logo}
            />
            <p className={styles.tagline}>
              A free, open-source generative-art software
            </p>
            <div className={styles.socials}>
              <LinkExternal
                to="https://discord.com/invite/QEjqnEy4aT"
                className={styles.iconBtn}
              >
                <FaDiscord />
              </LinkExternal>
              <LinkExternal
                to="https://www.instagram.com/coollab_art"
                className={styles.iconBtn}
              >
                <FaInstagram />
              </LinkExternal>
              <LinkExternal
                to="https://mastodon.social/@Coollab"
                className={styles.iconBtn}
              >
                <FaMastodon />
              </LinkExternal>
              <LinkExternal
                to="https://github.com/Coollab-Art/Coollab"
                className={styles.iconBtn}
              >
                <FaGithub />
              </LinkExternal>
            </div>
          </div>

          {/* Liens */}
          <nav className={styles.links}>
            <div className={styles.col}>
              <h4>Explore</h4>
              <LinkInternal to="/Tutorials/Discovery/Intro">
                Tutorials
              </LinkInternal>
              <LinkInternal to="/Gallery">Gallery</LinkInternal>
              <LinkInternal to="/Contribute">Contribute</LinkInternal>
            </div>
            <div className={styles.col}>
              <h4>Community</h4>
              <LinkExternal to="https://linktr.ee/coollab_art">
                Linktree
              </LinkExternal>
              <LinkExternal to="https://github.com/Coollab-Art/Coollab/issues/new/choose">
                Report an issue
              </LinkExternal>
              <LinkExternal to="https://www.etsy.com/shop/CoollabArt">
                Merch
              </LinkExternal>
              <LinkExternal to="https://utip.io/coollab/">Donate</LinkExternal>
            </div>
          </nav>
        </div>

        {/* Ligne séparateur */}
        <div className={styles.hr} />

        {/* Bas de page */}
        <div className={styles.bottom}>
          <p className={styles.copy}>
            © {year} Coollab. Built with{" "}
            <LinkExternal to="https://docusaurus.io/">Docusaurus</LinkExternal>
          </p>
        </div>
      </div>
    </footer>
  )
}
