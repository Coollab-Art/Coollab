
Some nodes can output some values that can be used by the nodes that come before them. This is typically used by nodes that duplicate their input, so that said input can change between each iteration / instantiation.<br/>
Such outputs are declared with the `OUTPUT` keyword, must have a name that starts and ends with single quotes (`'`) and must be of type `float`[^1]. If your output is an int, just cast it to a float. If your output is a vector, just declare several outputs.

[^1]: If you have a use case for outputs that are not of type float, feel free to [contact us](https://github.com/Coollab-Art/Coollab/issues/new?assignees=&labels=enhancement%2Ctriage&template=feature-suggestion.yaml&title=%5BFeature%5D+) and we will happily consider your use case and improve Coollab to make it fit your needs.

You can then simply assign to your output whenever it needs to change.

```glsl title="Render N Times.clbnode"
INPUT UV->Oklab_PremultipliedA 'Image';
INPUT int 'N';
INPUT(Oklab_PremultipliedA, Oklab_PremultipliedA)->Oklab_PremultipliedA 'Blend Mode';

OUTPUT float 'Index';

Oklab_PremultipliedA main(UV uv)
{
    vec4 color = vec4(0.);

    for (int i = 0; i < 'N'; ++i)
    {
        'Index' = float(i); // Set 'Index' before calling our INPUT 'Image' function that might depend on that 'Index'.
        color   = 'Blend Mode'('Image'(uv), color);
    }

    return color;
}
```

:::caution
Setting the current index should be the first thing you do before calling any of your INPUT functions. This is very important in order to ensure they will see the correct index when they execute.
:::