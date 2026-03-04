import React from "react"
import styles from "./styles.module.css"
import {
  FaDiscord,
  FaInstagram,
  FaLinkedin,
  FaMastodon,
  FaGithub,
  FaEnvelope,
} from "react-icons/fa"
import LinkExternal from "../../components/LinkExternal"
import LinkInternal from "../../components/LinkInternal"

export default function Footer() {
  const year = new Date().getFullYear()

  return (
    <footer className={styles.footer}>
      <div className={styles.container}>
        {}
        <div className={styles.top}>
          <div>
            <LinkInternal to="/">
              <img
                src="/img/logo-text.webp"
                alt="Coollab Logo"
                className={styles.logo}
              />
            </LinkInternal>
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
                to="https://www.linkedin.com/company/coollab-art"
                className={styles.iconBtn}
              >
                <FaLinkedin />
              </LinkExternal>
              <LinkExternal
                to="https://github.com/Coollab-Art/Coollab"
                className={styles.iconBtn}
              >
                <FaGithub />
              </LinkExternal>
              <LinkExternal
                to="mailto:coollab.lib@gmail.com"
                className={styles.iconBtn}
              >
                <FaEnvelope />
              </LinkExternal>
            </div>
          </div>

          {/* Links */}
          <nav className={styles.links}>
            <div className={styles.col}>
              <h4>Tutorials</h4>
              <LinkInternal to="/Tutorials">Getting Started</LinkInternal>
              <LinkInternal to="/Tutorials/VJ/Output%20window">VJ</LinkInternal>
              <LinkInternal to="/Tutorials/Writing%20Nodes/Intro">
                Writing nodes
              </LinkInternal>
            </div>
            <div className={styles.col}>
              <h4>Contribute</h4>
              <LinkInternal to="/Contribute/Feedback">Feedback</LinkInternal>
              <LinkInternal to="/Contribute/Outreach">Outreach</LinkInternal>
              <LinkInternal to="/Contribute/Code">Code</LinkInternal>
              <LinkInternal to="/Contribute/Design">Design</LinkInternal>
              <LinkInternal to="/Contribute/Donations">Donations</LinkInternal>
            </div>
            <div className={styles.col}>
              <h4>Install</h4>
              <LinkInternal to="/Download">Download</LinkInternal>
            </div>
          </nav>
        </div>

        {/* Separator line */}
        <div className={styles.hr} />

        {/* Bottom */}
        <div className={styles.bottom}>
          <p className={styles.copy}>
            © {year} Coollab. Built with{" "}
            <LinkExternal to="https://docusaurus.io/">Docusaurus</LinkExternal>.{" "}
            Work illustrations by{" "}
            <LinkExternal to="https://storyset.com/work">Storyset</LinkExternal>
            .
          </p>
        </div>
      </div>
    </footer>
  )
}
