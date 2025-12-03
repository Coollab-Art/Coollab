import React from "react"
import Layout from "@theme/Layout"
import ContributeCards from "../components/ContributeCards"
import ContributorsList from "../components/ContributorsList"
import LinkExternal from "../components/LinkExternal"
import SectionTitle from "../components/SectionTitle"
import Spacer from "../components/Spacer"

export default function ContributePage() {
  return (
    <Layout
      title="Contribute"
      description="Want to join the adventure? Here is how you can contribute to Coollab"
    >
      <main>
        <div style={{ padding: "8rem 2rem 4em 2em" }}>
          <SectionTitle fontSize="3.5rem" mobileFontSize="2.5rem">
            How to contribute:
          </SectionTitle>
          <Spacer height="2rem"></Spacer>
          <ContributeCards />
        </div>
        <div style={{ padding: "4rem 2rem 4em 2em" }}>
          <SectionTitle fontSize="3rem" mobileFontSize="1.7rem">
            Contributors
          </SectionTitle>
          <Spacer height="2rem"></Spacer>
          <p>
            This project was initiated by{" "}
            <LinkExternal to="https://julesfouchy.github.io/">
              Jules Fouchy
            </LinkExternal>{" "}
            and made possible by all our lovely contributors:
          </p>
          <ContributorsList />
        </div>
      </main>
    </Layout>
  )
}
