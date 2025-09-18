import React from "react"
import { Card, CardList } from "./CardList"
import {
  FaBullhorn,
  FaCommentDots,
  FaBug,
  FaUserSecret,
  FaPaintBrush,
  FaChalkboardTeacher,
  FaCodeBranch,
  FaCode,
  FaDonate,
} from "react-icons/fa"

export default function () {
  return (
    <CardList /* compactCards */ centered>
      <Card icon={<FaBullhorn />} internalLink="/">
        Tell your friends about <b>Coollab</b>!
      </Card>
      <Card
        icon={<FaCommentDots />}
        externalLink="https://github.com/Coollab-Art/Coollab/issues/new?labels=enhancement"
      >
        Share your <b>feedback</b> and <b>ideas</b>.
      </Card>
      <Card
        icon={<FaBug />}
        externalLink="https://github.com/Coollab-Art/Coollab/issues/new?labels=bug"
      >
        Report <b>bugs</b> you encounter.
      </Card>
      <Card icon={<FaUserSecret />}>
        Help us <b>beta-test</b> new features.
      </Card>
      <Card icon={<FaCommentDots />}>
        Give your <b>opinion</b> on design questions.
      </Card>
      <Card icon={<FaChalkboardTeacher />}>
        Make <b>tutorials</b> or <b>quick tip</b> videos.
      </Card>
      <Card
        icon={<FaCodeBranch />}
        internalLink="/Tutorials/Writing%20Nodes/Intro"
      >
        Write and share your own <b>nodes</b>.
      </Card>
      <Card
        icon={<FaPaintBrush />}
        externalLink="https://www.instagram.com/coollab_art/"
      >
        Share your <b>artworks</b> made with Coollab!
      </Card>
      <Card
        icon={<FaCode />}
        externalLink="https://github.com/Coollab-Art/Coollab/issues?q=label%3A%22good+first+issue%22"
      >
        Contribute to Coollab's <b>codebase</b>.
      </Card>
      <Card
        icon={<FaDonate />}
        externalLink="https://www.etsy.com/shop/CoollabArt"
      >
        <b>Support</b> us financially or buy <b>T-shirts</b>.
      </Card>
    </CardList>
  )
}
