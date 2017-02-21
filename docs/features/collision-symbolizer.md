# `CollisionSymbolizer`

It's sometimes convinient to "render" some invisible labels and insert them into collision detector in order to protect their places agains other objects. This can be done by `TextSymbolizer` or `MarkersSymbolizer` with `opacity="0"` but it comes with unnecessary performance overhead of text or marker rendering and seems like hackish solution in general.  The `CollisionSymbolizer` solves that.

## An example

[collision-symbolizer-4.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/collision-symbolizer-4.xml)

![collision-symbolizer-4](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/collision-symbolizer-4-800-800-1.0-agg-reference.png)

