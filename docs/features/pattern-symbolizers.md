# Pattern symbolizers improvements

## Better SVG support

Both `LinePatternSymbolizer` and `PolygonPatternSymbolizer` can customize SVG pattern by parameters

* `fill`
* `fill-opacity`
* `stroke`
* `stroke-width`
* `stroke-opacity`

### An example

[polygon-pattern-2.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/polygon-pattern-2.xml)

![polygon-pattern-2](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/polygon-pattern-2-256-256-1.0-agg-reference.png)

## `spacing`

Parameter `spacing` of `PolygonPatternSymbolizer` sets spacing between individual images of given pattern.

### An example

[polygon-pattern-6.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/polygon-pattern-6.xml)

![polygon-pattern-6](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/polygon-pattern-6-256-256-1.0-agg-reference.png)

## `lacing`

Parameter `lacing` of `PolygonPatternSymbolizer` set to `alternating-grid` will render pattern resembling bricks in a wall. Value `grid` is the default when parameter is unset.

### An example

[polygon-pattern-4.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/polygon-pattern-4.xml)

![polygon-pattern-4](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/polygon-pattern-4-256-256-1.0-agg-reference.png)


