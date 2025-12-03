:::caution
**🚧 WIP 🚧**
:::

## Curves

Use a Curve Renderer and lower the number of segments.

## Caching

The "Caching" node can help the performance a lot in some cases (e.g. when putting it before a Chromatic Aberration, a Glow, or any other node that needs to sample its input several times, i.e. nodes that have a Quality / Count / Iterations parameter). What the Caching node does is render an image of everything that comes before it, and pass it to the next nodes. This can prevent recomputing complex nodes several times, but can also lead to some visual artifacts (i.e. a Translate would need information that is outside of the cached image and will not be able to get it)

![](https://github.com/user-attachments/assets/75f613a5-2fcb-4b2a-a697-824d258e82d6)