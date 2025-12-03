import React from "react"
import { Card, CardList } from "./CardList"
import {
  FaExchangeAlt,
  FaVideo,
  FaMusic,
  FaCode,
  FaWifi,
  FaKeyboard,
} from "react-icons/fa"

export default function () {
  return (
    // TODO(Website) maybe we need images to illustrate these cards
    <div style={{ paddingLeft: "75px", paddingRight: "75px" }}>
      <CardList centered>
        <Card icon={<FaMusic />} internalLink="/Tutorials/Features/Audio">
          <b>Audio</b> reactive visuals
        </Card>
        <Card icon={<FaVideo />}>
          <b>Webcam</b> integration
        </Card>
        <Card icon={<FaKeyboard />}>
          <b>Particles</b>
        </Card>
        <Card
          icon={<FaExchangeAlt />}
          internalLink="/Tutorials/VJing#spout-out"
        >
          {/* TODO(Website) use the official Spout logo ? put in white ? */}
          <b>Spout</b> IN & OUT
        </Card>
        <Card icon={<FaKeyboard />}>
          <b>MIDI</b> control
        </Card>
        <Card icon={<FaWifi />}>
          <b>OSC</b> support
        </Card>
        <Card icon={<FaCode />} internalLink="/Tutorials/Writing Nodes/Intro">
          Custom <b>GLSL</b> nodes
        </Card>
      </CardList>
    </div>
  )
}
