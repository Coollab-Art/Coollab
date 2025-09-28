// @ts-check
// Note: type annotations allow type checking and IDEs autocompletion

const lightCodeTheme = require("prism-react-renderer/themes/github")
const darkCodeTheme = require("prism-react-renderer/themes/dracula")

const math = require("remark-math")
const katex = require("rehype-katex")

const organizationName = "Coollab-Art"
const projectName = "Lab"

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: "Coollab",
  tagline: "A free generative-art software",
  favicon: "img/favicon.ico",

  // Set the production url of your site here
  url: `https://coollab-art.com/`,
  // Set the /<baseUrl>/ pathname under which your site is served
  // For GitHub pages deployment, it is often '/<projectName>/'
  baseUrl: `/`,

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName, // Usually your GitHub org/user name.
  projectName, // Usually your repo name.
  trailingSlash: false,

  onBrokenLinks: "throw",
  onBrokenMarkdownLinks: "throw",

  // Even if you don't use internalization, you can use this field to set useful
  // metadata like html lang. For example, if your site is Chinese, you may want
  // to replace "en" with "zh-Hans".
  i18n: {
    defaultLocale: "en",
    locales: ["en"],
  },

  presets: [
    [
      "classic",
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          path: "content",
          routeBasePath: "/",
          sidebarPath: require.resolve("./sidebars.js"),
          remarkPlugins: [math],
          rehypePlugins: [katex],
        },
        theme: {
          customCss: [
            require.resolve("./src/css/custom.css"),
            require.resolve("./src/css/icomoon.css"),
          ],
        },
      }),
    ],
  ],

  plugins: [require.resolve("docusaurus-lunr-search")],

  stylesheets: [
    {
      href: "https://cdn.jsdelivr.net/npm/katex@0.13.24/dist/katex.min.css",
      type: "text/css",
      integrity:
        "sha384-odtC+0UGzzFL/6PNoE8rX/SPcQDXBJ+uRepguP4QkPCm2LBxH3FA3y+fKSiJ+AmM",
      crossorigin: "anonymous",
    },
    {
      href: "https://maxcdn.bootstrapcdn.com/font-awesome/4.7.0/css/font-awesome.min.css",
      type: "text/css",
      integrity:
        "sha384-wvfXpqpZZVQGK6TAh5PVlGOfQNHSoD2xbE+QkPxCAFlNEevoEH3Sl0sibVcOQVnN",
      crossorigin: "anonymous",
    },
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      // Replace with your project's social card
      image: "img/logo.png",
      colorMode: {
        defaultMode: "dark",
        disableSwitch: false,
        respectPrefersColorScheme: false,
      },
      navbar: {
        title: "",
        logo: {
          alt: "Coollab Logo",
          src: "img/logo.png",
        },
        items: [
          {
            type: "doc",
            docId: "Tutorials/Discovery/Intro",
            position: "left",
            label: "Tutorials",
          },
          {
            type: "doc",
            docId: "Download/download",
            position: "left",
            label: "Download",
          },
          {
            to: "Gallery",
            position: "left",
            label: "Gallery",
          },
          {
            to: "Contribute",
            position: "left",
            label: "Contribute",
          },
          {
            type: "doc",
            docId: "Articles/Alpaca/Alpaca",
            position: "right",
            label: "Articles",
          },
          {
            type: "doc",
            docId: "About",
            position: "right",
            label: "About",
          },
        ],
      },
      footer: {
        style: "dark",
        links: [
          {
            title: "Find Us",
            items: [
              {
                label: "LinkTree",
                to: "https://linktr.ee/coollab_art",
              },
              {
                label: "GitHub",
                to: "https://github.com/Coollab-Art/Coollab",
              },
            ],
          },
          {
            title: "Raise an issue!",
            items: [
              {
                label: "Report a bug",
                to: "https://github.com/Coollab-Art/Coollab/issues/new?assignees=&labels=bug%2Ctriage&template=bug-report.yaml&title=%5BBug%5D+",
              },
              {
                label: "Give us feedback",
                to: "https://github.com/Coollab-Art/Coollab/issues/new?assignees=&labels=enhancement%2Ctriage&template=feature-suggestion.yaml&title=%5BFeature%5D+",
              },
            ],
          },
          {
            title: "Say thank you",
            items: [
              {
                label: "Send a nice message",
                to: "https://www.instagram.com/coollab_art/",
              },
              {
                label: "Leave a like",
                to: "https://www.instagram.com/coollab_art/",
              },
              {
                label: "Make a donation",
                to: "https://utip.io/coollab/",
              },
            ],
          },
        ],
        copyright: `Copyright © ${new Date().getFullYear()}. Built with <a href="https://docusaurus.io/">Docusaurus</a>.
                    <div>Icons made by <a href="https://www.freepik.com" title="Freepik">Freepik</a> from <a href="https://www.flaticon.com/" title="Flaticon">www.flaticon.com</a>.</div>
                    `,
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
        additionalLanguages: ["glsl", "cmake"],
      },
    }),
}

module.exports = config
