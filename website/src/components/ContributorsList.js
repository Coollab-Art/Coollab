import React from "react"
import RandomizedList from "./RandomizedList"
import Contributor from "./Contributor"
import { CenterText } from "../components/CenterH"
import style from "./ContributorsList.module.css"

export default function ContributorsList({ category }) {
  const filterByCategory = (contributors) => {
    if (!category) return contributors

    return contributors.filter(
      (contributor) =>
        contributor.props.categories &&
        contributor.props.categories.includes(category),
    )
  }

  return (
    <>
      <CenterText>
        ✨
        {category === "Donations" ? (
          <b class="gradient-text">Many thanks to all our generous donors!</b>
        ) : category ? (
          <b class="gradient-text">
            Many thanks to all our {category} contributors!
          </b>
        ) : (
          <b class="gradient-text">
            Many thanks to all our lovely contributors!
          </b>
        )}
        ✨
      </CenterText>
      <div className={style.contributorsList}>
        <RandomizedList
          elements={filterByCategory([
            <Contributor
              name="Jules Fouchy"
              github_name="julesfouchy"
              link="https://julesfouchy.github.io/"
              roles={["Project Lead", "Programmer"]}
              categories={["Code", "Design", "Outreach"]}
            />,
            <Contributor
              name="Jolan Goulin"
              github_name="Jolan-Goulin"
              link="https://github.com/Jolan-Goulin"
              roles={["Programmer (3D Shapes)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Emma Pernin"
              github_name="Emma-prn"
              link="https://github.com/Emma-prn"
              roles={["Programmer (3D Shapes)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Théo Arnauld des Lions"
              github_name="Dyokiin"
              link="https://github.com/Dyokiin"
              roles={["Programmer (3D Shapes)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Quentin Mesquita"
              github_name="MesQuentin"
              link="https://github.com/MesQuentin"
              roles={["Programmer (3D Shapes)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Lucas David"
              github_name="MeucheRoume"
              link="http://lucasdavid.fr/"
              roles={["Programmer (Search bar)", "Donation"]}
              categories={["Code", "Donations"]}
            />,
            <Contributor
              name="Anaïs Sarata Gougne"
              github_name="TaliaKah"
              link="https://linktr.ee/taliakah"
              roles={[
                "Programmer (Color Ramp)",
                "Beta-tester",
                "VJ",
                "Outreach",
                "Donation",
              ]}
              categories={["Code", "Donations", "Feedback", "Outreach"]}
            />,
            <Contributor
              name="Enguerrand de Smet"
              github_name="dsmtE"
              link="https://github.com/dsmtE"
              roles={["Programmer (3D Mesh Export (WIP))"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Jordan Roberty"
              github_name="JordanRoberty"
              link="https://github.com/JordanRoberty"
              roles={["Programmer (Porting Coollab to MacOS, CI scripts)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Éva Benharira"
              github_name="EvaBien"
              link="https://www.linkedin.com/in/eva-benharira-ingenieure-creative/"
              roles={["Gave me some advice on community management"]}
              categories={["Outreach"]}
            />,
            <Contributor
              name="Mattéo Leclercq"
              github_name="MatteoL-W"
              link="https://matteo-leclercq.fr/"
              roles={["Programmer (2D Nodes)", "Demo projects"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Wendy Gervais"
              github_name="kaugrv"
              link="https://linktr.ee/commandant.grant"
              roles={[
                "Programmer (2D Nodes)",
                "1st Trailer",
                "Demo projects",
                "Beta-tester",
              ]}
              categories={["Code", "Feedback", "Design"]}
            />,
            <Contributor
              name="Romain Serres"
              github_name="Bouillon2Poulet"
              link="https://www.instagram.com/consomme2poyo/"
              roles={["Programmer (2D Nodes)", "Demo projects", "Coollab logo"]}
              categories={["Code", "Design"]}
            />,
            <Contributor
              name="Lou Couard"
              github_name="Couloir00"
              link="https://loucouard.wixsite.com/website"
              roles={[
                "Programmer (2D Nodes)",
                "1st Trailer",
                "2nd Trailer",
                "Demo projects",
              ]}
              categories={["Code", "Design"]}
            />,
            <Contributor
              name="Tristan Debeaune"
              github_name="tristetemps666"
              link="https://github.com/tristetemps666"
              roles={[
                "Programmer (2D Nodes, Webcam)",
                "1st Trailer",
                "Demo projects",
              ]}
              categories={["Code", "Design"]}
            />,
            <Contributor
              name="Amandine Kohlmuller"
              pictureSrc="/img/amandine.webp"
              link="https://idnamaa.fun"
              roles={[
                "UI / UX Designer",
                "Helped with Web Design",
                "Coollab logo",
              ]}
              categories={["Design"]}
            />,
            <Contributor
              name="Aurore Lafaurie"
              github_name="Just-Kiel"
              link="https://github.com/Just-Kiel"
              roles={['Programmer ("Did you know?" tips)']}
              categories={["Code"]}
            />,
            <Contributor
              name="Théo Couard"
              github_name="TheoCrd"
              link="https://frankoo.artstation.com/"
              roles={["Video tutorials (Discovery series)"]}
              categories={["Outreach"]}
            />,
            <Contributor
              name="Philippe Rosales"
              github_name="Audiophil15"
              link="https://github.com/Audiophil15"
              roles={["Programmer (Audio)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Bruce Lane"
              github_name="brucelane"
              link="https://www.linkedin.com/in/brucelane06/"
              roles={["Programmer (MIDI)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Marianne Kerckhove"
              github_name="Maria30"
              link="https://www.mariannekerckhove.com/"
              roles={["Programmer (Particles)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Baptiste Jouin"
              github_name="baptistejouin"
              link="https://github.com/baptistejouin"
              roles={["Programmer (Particles)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Thomas Garnier"
              github_name="TotoShampoin"
              link="https://github.com/TotoShampoin"
              roles={["Programmer (Particles)", "Feedback"]}
              categories={["Code", "Feedback"]}
            />,
            <Contributor
              name="Mattéo Moulat"
              github_name="Xiantas"
              link="https://github.com/Xiantas"
              roles={["Programmer (Particles)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Théo Bonnot"
              github_name="fochyl"
              link="https://github.com/fochyl"
              roles={["Programmer (Webcam)", "Short Tutorials"]}
              categories={["Code", "Outreach"]}
            />,
            <Contributor
              name="Agathe Jan"
              github_name="Agwathe"
              link="https://github.com/Agwathe"
              roles={["Programmer (Webcam)", "Speed Art videos"]}
              categories={["Code", "Outreach"]}
            />,
            <Contributor
              name="Aurélien Michaud"
              github_name="AurelienMichaud"
              link="https://github.com/AurelienMichaud"
              roles={["Short Tutorials"]}
              categories={["Outreach"]}
            />,
            <Contributor
              name="Marie Jacquelin"
              github_name="MarieJcqln"
              link="https://github.com/MarieJcqln"
              roles={["Programmer (Webcam)", "Speed Art videos"]}
              categories={["Code", "Outreach"]}
            />,
            <Contributor
              name="Maxence Dupuis"
              github_name="smallboyc"
              link="https://github.com/smallboyc"
              roles={["Programmer (Launcher, Webcam)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Anna-Maria Lannaud"
              github_name="AM-XIX"
              link="https://github.com/AM-XIX"
              roles={["Website (Design and Code)"]}
              categories={["Design", "Code"]}
            />,
            <Contributor
              name="Elvin Kauffmann"
              github_name="ShadowsHood"
              link="https://www.linkedin.com/in/elvin-kauffmann-754172213"
              roles={["Programmer (Regression tests, Websocket API)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Anass Doublal"
              github_name="AnassDoublal"
              link="https://github.com/AnassDoublal"
              roles={["Programmer (Launcher improvements)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Jari"
              pictureSrc="https://cdn.discordapp.com/avatars/1004260219058131024/72163e0366d6d0400e3ca8615157e02e?size=256"
              roles={["Sharing projects", "Feedback"]}
              categories={["Feedback"]}
            />,
            <Contributor
              name="SpeeQz1"
              pictureSrc="https://cdn.discordapp.com/avatars/880825377033973780/181ffb92ae6acdf04b56abfff142fa6d?size=256"
              link="https://github.com/SpeeQz1"
              roles={["Feedback", "Interesting design discussions"]}
              categories={["Feedback"]}
            />,
            <Contributor
              name="Peckham Digital"
              pictureSrc="/img/peckham-digital.webp"
              link="https://www.peckhamdigital.org/"
              roles={["Invited me for a workshop"]}
              categories={["Outreach"]}
            />,
            <Contributor
              name="WilDev"
              pictureSrc="/img/logo_wildev.svg"
              link="https://github.com/WilfriedEugster"
              roles={["Programmer (Nodes)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Emilie Desombre"
              pictureSrc="/img/emilie_des.png"
              link="https://linkedin.com/in/emilie-desombre-6b86702a9"
              roles={["Programmer (Nodes)"]}
              categories={["Code"]}
            />,
            <Contributor
              name="Goubeline"
              github_name="Goubeline"
              link="https://github.com/Goubeline"
              roles={["Programmer (Nodes)"]}
              categories={["Code"]}
            />,
          ])}
        />
      </div>
    </>
  )
}
