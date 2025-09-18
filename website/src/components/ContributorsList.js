import React from "react"
import RandomizedList from "./RandomizedList"
import Contributor from "./Contributor"
import style from "./ContributorsList.module.css"

export default function ContributorsList() {
  return (
    <div className={style.contributorsList}>
      <RandomizedList
        elements={[
          <Contributor
            name="Jules Fouchy"
            github_name="julesfouchy"
            link="https://julesfouchy.github.io/"
            roles={["Project Lead", "Programmer"]}
          />,
          <Contributor
            name="Jolan Goulin"
            github_name="Jolan-Goulin"
            link="https://github.com/Jolan-Goulin"
            roles={["Programmer (3D Shapes)"]}
          />,
          <Contributor
            name="Emma Pernin"
            github_name="Emma-prn"
            link="https://github.com/Emma-prn"
            roles={["Programmer (3D Shapes)"]}
          />,
          <Contributor
            name="Théo Arnauld des Lions"
            github_name="Dyokiin"
            link="https://github.com/Dyokiin"
            roles={["Programmer (3D Shapes)"]}
          />,
          <Contributor
            name="Quentin Mesquita"
            github_name="MesQuentin"
            link="https://github.com/MesQuentin"
            roles={["Programmer (3D Shapes)"]}
          />,
          <Contributor
            name="Lucas David"
            github_name="MeucheRoume"
            link="http://lucasdavid.fr/"
            roles={["Programmer (Search bar)"]}
          />,
          <Contributor
            name="Anaïs Sarata Gougne"
            github_name="TaliaKah"
            link="https://taliakah.gitlab.io/portfolio/"
            roles={["Programmer (Color Ramp)", "Beta-Tester", "VJ", "Outreach"]}
          />,
          <Contributor
            name="Enguerrand de Smet"
            github_name="dsmtE"
            link="https://github.com/dsmtE"
            roles={["Programmer (3D Mesh Export (WIP))"]}
          />,
          <Contributor
            name="Jordan Roberty"
            github_name="JordanRoberty"
            link="https://github.com/JordanRoberty"
            roles={["Programmer (Porting Coollab to MacOS, CI scripts)"]}
          />,
          <Contributor
            name="Éva Benharira"
            github_name="EvaBien"
            link="https://www.linkedin.com/in/eva-benharira-ingenieure-creative/"
            roles={["Gave me some advice for community management"]}
          />,
          <Contributor
            name="Mattéo Leclercq"
            github_name="MatteoL-W"
            link="https://matteo-leclercq.fr/"
            roles={["Programmer (2D Nodes)", "Demo projects"]}
          />,
          <Contributor
            name="Wendy Gervais"
            github_name="kaugrv"
            link="https://linktr.ee/commandant.grant"
            roles={["Programmer (2D Nodes)", "1st Trailer", "Demo projects"]}
          />,
          <Contributor
            name="Romain Serres"
            github_name="Bouillon2Poulet"
            link="https://www.instagram.com/consomme2poyo/"
            roles={["Programmer (2D Nodes)", "Demo projects"]}
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
          />,
          <Contributor
            name="Amandine Kohlmuller"
            github_name="akohlmul"
            link="https://www.instagram.com/idnamaa/"
            roles={["UI / UX Designer, Gave us some advice on Web Design"]}
          />,
          <Contributor
            name="Aurore Lafaurie"
            github_name="Just-Kiel"
            link="https://github.com/Just-Kiel"
            roles={['Programmer ("Did you know?" tips)']}
          />,
          <Contributor
            name="Théo Couard"
            github_name="TheoCrd"
            link="https://frankoo.artstation.com/"
            roles={["Video tutorials (Discovery series)"]}
          />,
          <Contributor
            name="Philippe Rosales"
            github_name="Audiophil15"
            link="https://github.com/Audiophil15"
            roles={["Programmer (Audio)"]}
          />,
          <Contributor
            name="Bruce Lane"
            github_name="brucelane"
            link="https://www.linkedin.com/in/brucelane06/"
            roles={["Programmer (MIDI)"]}
          />,
          <Contributor
            name="Marianne Kerckhove"
            github_name="Maria30"
            link="https://www.mariannekerckhove.com/"
            roles={["Programmer (Particles)"]}
          />,
          <Contributor
            name="Baptiste Jouin"
            github_name="baptistejouin"
            link="https://github.com/baptistejouin"
            roles={["Programmer (Particles)"]}
          />,
          <Contributor
            name="Thomas Garnier"
            github_name="TotoShampoin"
            link="https://github.com/TotoShampoin"
            roles={["Programmer (Particles)"]}
          />,
          <Contributor
            name="Mattéo Moulat"
            github_name="Xiantas"
            link="https://github.com/Xiantas"
            roles={["Programmer (Particles)"]}
          />,
          <Contributor
            name="Théo Bonnot"
            github_name="fochyl"
            link="https://github.com/fochyl"
            roles={["Programmer (Webcam)", "Short Tutorials"]}
          />,
          <Contributor
            name="Agathe Jan"
            github_name="Agwathe"
            link="https://github.com/Agwathe"
            roles={["Programmer (Webcam)", "Speed Art videos"]}
          />,
          <Contributor
            name="Aurélien Michaud"
            github_name="AurelienMichaud"
            link="https://github.com/AurelienMichaud"
            roles={["Short Tutorials"]}
          />,
          <Contributor
            name="Marie Jacquelin"
            github_name="MarieJcqln"
            link="https://github.com/MarieJcqln"
            roles={["Programmer (Webcam)", "Speed Art videos"]}
          />,
          <Contributor
            name="Maxence Dupuis"
            github_name="smallboyc"
            link="https://github.com/smallboyc"
            roles={["Programmer (Launcher, Webcam)"]}
          />,
          <Contributor
            name="Anna-Maria Lannaud"
            github_name="AM-XIX"
            link="https://github.com/AM-XIX"
            roles={["Website (Design and Code)"]}
          />,
          <Contributor
            name="Elvin Kauffmann"
            github_name="ShadowsHood"
            link="https://www.linkedin.com/in/elvin-kauffmann-754172213"
            roles={["Programmer (Regression tests, Websocket API)"]}
          />,
          <Contributor
            name="Anass Doublal"
            github_name="AnassDoublal"
            link="https://github.com/AnassDoublal"
            roles={["Programmer (Launcher improvements)"]}
          />,
        ]}
      />
    </div>
  )
}
