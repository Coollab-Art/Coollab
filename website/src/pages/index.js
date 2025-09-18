import React from "react"
import Layout from "@theme/Layout"
import Spacer from "../components/Spacer"
import FeaturesList from "../components/FeaturesList"
import Timeline from "../components/Timeline"
import GalleryPreview from "../components/GalleryPreview"
import YoutubeVideo from "../components/YoutubeVideo"
import ArgumentsCards from "../components/ArgumentsCards"
import Hero from "../components/Hero"
import SocialsBigButtons from "../components/SocialsBigButtons"
import SectionTitle from "../components/SectionTitle"
import CenterH from "../components/CenterH"

export default function Home() {
  return (
    <Layout title="" description="Node-based creation for everyone">
      <main>
        <Hero />
        <Spacer height="5rem" />
        <ArgumentsCards />
        <Spacer height="5rem" />
        <FeaturesList />
        <Spacer height="5rem" />
        <YoutubeVideo /* Trailer */ id="jsjkjCQCPno" />
        <Spacer height="5rem" />
        <CenterH>
          <SectionTitle fontSize="2.5rem" mobileFontSize="1.7rem">
            Unique and Powerful effects
          </SectionTitle>
        </CenterH>
        <Spacer height="2.4rem"></Spacer>
        <CenterH>
          <Timeline />
        </CenterH>
        <Spacer height="5rem" />
        {/* <SectionTitle fontSize="2.5rem" mobileFontSize="1.7rem">
          Interactivity, Protocols
        </SectionTitle>
        <Spacer height="2.4rem"></Spacer>
        OSC, Midi,Spout */}
        <CenterH>
          <SectionTitle fontSize="2.5rem" mobileFontSize="1.7rem">
            Follow us
          </SectionTitle>
        </CenterH>
        <Spacer height="2.4rem"></Spacer>
        <SocialsBigButtons />
        <Spacer height="5rem" />
        <GalleryPreview />
        <Spacer height="5rem" />
        <YoutubeVideo /* What is Coollab? */ id="nkKj_MTBBRI" />
        <Spacer height="5rem" />
      </main>
    </Layout>
  )
}
