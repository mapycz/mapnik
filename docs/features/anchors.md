
# Anchors

Allows to tie features across symbolizers, styles and layers. Once two features are connected, their mutual position and other properties can be adjusted.

Every symbolizer can have `key` property, whose value will be unique feature id in most cases. For example this MarkersSymbolizer:

```xml
<MarkersSymbolizer key="[id]" />

```

Another symbolizer can connect to the marker by `anchor-key` property. In this case of simple point text placement, `dx` and `dy` properties of the text symbolizer will be set according to the size of the marker so one doesn't have to adjust these properties if size of the marker changes:

```xml
<TextSymbolizer
    placement="point"
    placement-type="simple"
    anchor-key="[id]"
    >
</TextSymbolizer>

```

## Supported properties

Whereas all symbolizers can set their `key` property, only point symbolizer and markers symbolizer can offer its size and only text symbolizer can receive it by `anchor-key` currently.

However, anchors are meant to be universal tool for connections between features and more possibilities will come.

## Examples

### Marker with text on point with simple placement

[simple-anchor.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/simple-anchor.xml)

![simple-anchor](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/simple-anchor-500-100-1.0-agg-reference.png)

### Marker with text on point with angle placement

[angle-anchor-tolerance.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/angle-anchor-tolerance.xml)

![angle-anchor-tolerance](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/angle-anchor-tolerance-400-400-1.0-agg-reference.png)
