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

  plugins: [
    function addMetaTags() {
      return {
        name: "add-meta-tags",
        injectHtmlTags() {
          const siteUrl = "https://coollab-art.com"
          const title = "Coollab — Open-source generative-art software"
          const description =
            "A powerful FREE software for generating visual effects and motion graphics for installations, live performances, and VJing. Easy to learn, yet incredibly powerful."

          return {
            headTags: [
              // Open Graph
              {
                tagName: "meta",
                attributes: { property: "og:type", content: "website" },
              },
              {
                tagName: "meta",
                attributes: { property: "og:site_name", content: "Coollab" },
              },
              {
                tagName: "meta",
                attributes: { property: "og:title", content: title },
              },
              {
                tagName: "meta",
                attributes: {
                  property: "og:description",
                  content: description,
                },
              },
              {
                tagName: "meta",
                attributes: { property: "og:url", content: siteUrl },
              },

              // X
              {
                tagName: "meta",
                attributes: {
                  name: "twitter:card",
                  content: "summary_large_image",
                },
              },
              {
                tagName: "meta",
                attributes: { name: "twitter:title", content: title },
              },
              {
                tagName: "meta",
                attributes: {
                  name: "twitter:description",
                  content: description,
                },
              },

              // Divers
              {
                tagName: "link",
                attributes: { rel: "canonical", href: siteUrl },
              },
              {
                tagName: "meta",
                attributes: { name: "theme-color", content: "#27272A" },
              },
            ],
          }
        },
      }
    },
  ],
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
    {
      href: "https://fonts.googleapis.com/css2?family=League+Spartan:wght@400;700&family=Roboto:wght@400;700&display=swap",
      type: "text/css",
    },
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      image: "img/pagescreen-hero.png", // TODO(Website) use an actual screen with the navbar and wave included ? (still remove scrollbar on the right with overflow: hidden;)
      colorMode: {
        defaultMode: "dark",
        disableSwitch: true,
        respectPrefersColorScheme: false,
      },
      navbar: {
        hideOnScroll: false,
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
            to: "Gallery",
            position: "left",
            label: "Gallery",
          },
          {
            to: "Contribute",
            position: "left",
            label: "Contribute",
          },
          // TODO(Website)
          //   {
          //     type: "doc",
          //     docId: "Articles/Alpaca/Alpaca",
          //     position: "left",
          //     label: "Articles",
          //   },
          {
            type: "html",
            position: "right",
            value: `
    <div class="navbar-socials">
      <!-- Discord -->
      <a class="iconBtn" href="https://discord.com/invite/QEjqnEy4aT" target="_blank" rel="noopener noreferrer" aria-label="Discord">
        <svg width="20" height="20" xmlns="http://www.w3.org/2000/svg" shape-rendering="geometricPrecision" text-rendering="geometricPrecision" image-rendering="optimizeQuality" fill-rule="evenodd" clip-rule="evenodd" viewBox="0 0 512 365.467"><path fill="#fff" d="M378.186 365.028s-15.794-18.865-28.956-35.099c57.473-16.232 79.41-51.77 79.41-51.77-17.989 11.846-35.099 20.182-50.454 25.885-21.938 9.213-42.997 14.917-63.617 18.866-42.118 7.898-80.726 5.703-113.631-.438-25.008-4.827-46.506-11.407-64.494-18.867-10.091-3.947-21.059-8.774-32.027-14.917-1.316-.877-2.633-1.316-3.948-2.193-.877-.438-1.316-.878-1.755-.878-7.898-4.388-12.285-7.458-12.285-7.458s21.06 34.659 76.779 51.331c-13.163 16.673-29.395 35.977-29.395 35.977C36.854 362.395 0 299.218 0 299.218 0 159.263 63.177 45.633 63.177 45.633 126.354-1.311 186.022.005 186.022.005l4.388 5.264C111.439 27.645 75.461 62.305 75.461 62.305s9.653-5.265 25.886-12.285c46.945-20.621 84.236-25.885 99.592-27.64 2.633-.439 4.827-.878 7.458-.878 26.763-3.51 57.036-4.387 88.624-.878 41.68 4.826 86.43 17.111 132.058 41.68 0 0-34.66-32.906-109.244-55.281l6.143-7.019s60.105-1.317 122.844 45.628c0 0 63.178 113.631 63.178 253.585 0-.438-36.854 62.739-133.813 65.81l-.001.001zm-43.874-203.133c-25.006 0-44.75 21.498-44.75 48.262 0 26.763 20.182 48.26 44.75 48.26 25.008 0 44.752-21.497 44.752-48.26 0-26.764-20.182-48.262-44.752-48.262zm-160.135 0c-25.008 0-44.751 21.498-44.751 48.262 0 26.763 20.182 48.26 44.751 48.26 25.007 0 44.75-21.497 44.75-48.26.439-26.763-19.742-48.262-44.75-48.262z"/>
        </svg>
      </a>

      <!-- Instagram -->
      <a class="iconBtn" href="https://www.instagram.com/coollab_art" target="_blank" rel="noopener noreferrer" aria-label="Instagram">
        <svg width="20" height="20" viewBox="0 0 24 24" fill="currentColor" aria-hidden="true" focusable="false">
          <path d="M7 2h10a5 5 0 0 1 5 5v10a5 5 0 0 1-5 5H7a5 5 0 0 1-5-5V7a5 5 0 0 1 5-5Zm0 2a3 3 0 0 0-3 3v10a3 3 0 0 0 3 3h10a3 3 0 0 0 3-3V7a3 3 0 0 0-3-3H7Zm5 3.5A5.5 5.5 0 1 1 6.5 13 5.51 5.51 0 0 1 12 7.5Zm0 2A3.5 3.5 0 1 0 15.5 13 3.5 3.5 0 0 0 12 9.5Zm5.25-3.25a1.25 1.25 0 1 1-1.25 1.25 1.25 1.25 0 0 1 1.25-1.25Z"/>
        </svg>
      </a>

      <!-- Mastodon -->
      <a class="iconBtn" href="https://mastodon.social/@Coollab" target="_blank" rel="noopener noreferrer" aria-label="Mastodon">
        <svg width="20" height="20" xmlns="http://www.w3.org/2000/svg" shape-rendering="geometricPrecision" text-rendering="geometricPrecision" image-rendering="optimizeQuality" fill-rule="evenodd" clip-rule="evenodd" viewBox="0 0 480 511.475"><path fill="#fff" fill-rule="nonzero" d="M478.065 113.237c-7.393-54.954-55.29-98.266-112.072-106.656C356.413 5.162 320.121 0 236.043 0h-.626c-84.099 0-102.141 5.162-111.72 6.581C68.498 14.738 18.089 53.653 5.859 109.26c-5.882 27.384-6.51 57.747-5.416 85.594C2 234.794 2.302 274.662 5.93 314.438a562.647 562.647 0 0013.09 78.437c11.625 47.653 58.686 87.311 104.793 103.494a281.07 281.07 0 00153.315 8.09 223.879 223.879 0 0016.578-4.533c12.368-3.929 26.855-8.323 37.507-16.043a1.198 1.198 0 00.486-.928v-38.544a1.12 1.12 0 00-.437-.879 1.134 1.134 0 00-.957-.215 424.764 424.764 0 01-99.49 11.626c-57.665 0-73.172-27.361-77.611-38.75a120.008 120.008 0 01-6.745-30.549 1.048 1.048 0 01.094-.51c.07-.161.178-.304.317-.415a1.094 1.094 0 01.982-.213 417.004 417.004 0 0097.865 11.624c7.927 0 15.834 0 23.761-.213 33.156-.928 68.105-2.625 100.722-8.997.816-.16 1.63-.301 2.327-.508 51.455-9.883 100.423-40.894 105.397-119.421.184-3.092.652-32.382.652-35.588.021-10.904 3.509-77.344-.511-118.166zm-82.796 61.744V311.51h-54.104V179.003c0-27.897-11.627-42.126-35.273-42.126-25.998 0-39.018 16.834-39.018 50.073v72.534h-53.776V186.95c0-33.239-13.045-50.073-39.04-50.073-23.507 0-35.249 14.229-35.249 42.126V311.51H84.73V174.981c0-27.896 7.121-50.059 21.366-66.488 14.693-16.388 33.969-24.805 57.893-24.805 27.692 0 48.618 10.648 62.569 31.919l13.464 22.597 13.485-22.597c13.95-21.271 34.876-31.919 62.52-31.919 23.903 0 43.177 8.417 57.919 24.805 14.231 16.413 21.337 38.576 21.323 66.488z"/></svg>
      </a>
          <!-- Github -->
      <a class="iconBtn" href="https://github.com/Coollab-Art/Coollab" target="_blank" rel="noopener noreferrer" aria-label="Github">
      <svg width="20" height="20" version="1.1" id="Layer_1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" viewBox="0 0 291.32 291.32" xml:space="preserve" fill="#FFFFF" stroke="#FFFFF"><g id="SVGRepo_bgCarrier" stroke-whiteidth="0"></g><g id="SVGRepo_tracerCarrier" stroke-linecap="round" stroke-linejoin="round"></g><g id="SVGRepo_iconCarrier"> <g> <path style="fill:white;" d="M145.66,0C65.219,0,0,65.219,0,145.66c0,80.45,65.219,145.66,145.66,145.66 s145.66-65.21,145.66-145.66C291.319,65.219,226.1,0,145.66,0z M186.462,256.625c-0.838-11.398-1.775-25.518-1.83-31.235 c-0.364-4.388-0.838-15.549-11.434-22.677c42.068-3.523,62.087-26.774,63.526-57.499c1.202-17.497-5.754-32.883-18.107-45.3 c0.628-13.282-0.401-29.023-1.256-35.941c-9.486-2.731-31.608,8.949-37.79,13.947c-13.037-5.062-44.945-6.837-64.336,0 c-13.747-9.668-29.396-15.64-37.926-13.974c-7.875,17.452-2.813,33.948-1.275,35.914c-10.142,9.268-24.289,20.675-20.447,44.572 c6.163,35.04,30.816,53.94,70.508,58.564c-8.466,1.73-9.896,8.048-10.606,10.788c-26.656,10.997-34.275-6.791-37.644-11.425 c-11.188-13.847-21.23-9.832-21.849-9.614c-0.601,0.218-1.056,1.092-0.992,1.511c0.564,2.986,6.655,6.018,6.955,6.263 c8.257,6.154,11.316,17.27,13.2,20.438c11.844,19.473,39.374,11.398,39.638,11.562c0.018,1.702-0.191,16.032-0.355,27.184 C64.245,245.992,27.311,200.2,27.311,145.66c0-65.365,52.984-118.348,118.348-118.348S264.008,80.295,264.008,145.66 C264.008,196.668,231.69,239.992,186.462,256.625z"></path> </g> </g></svg>
      </a>
    </div>
  `,
          },
          {
            to: "/Download",
            position: "right",
            label: "Download",
          },
        ],
      },
      prism: {
        theme: lightCodeTheme,
        darkTheme: darkCodeTheme,
        additionalLanguages: ["glsl", "cmake"],
      },
    }),
}

module.exports = config
