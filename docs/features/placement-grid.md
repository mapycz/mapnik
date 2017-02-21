
# Placement `grid` and `alternating-grid`

Allows placing repeated labels on area of a polygon. All possible placements create grid of points. Only placements within given polygon are rendered.

`alternating-grid` creates pattern similar to bricks in a wall.

Very important is order in which grid is iterated and individual placements placed. The first point is polygon's interior. Subsequent placements goes around interior in a spiral.
Only CollisionSymbolizer does not work this way, but rather iterates the grid in left-right top-down order.

These are basic use cases:

* Labeling areas in given density.
* Trying various position around interior.
* Adding areas into collision detector by `CollisionSymbolizer`.

Grid density can be controlled by `grid-cell-width` and `grid-cell-height` properties of text symbolizer.

## An example

[grid-on-polygon-3.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/grid-on-polygon-3.xml)

![grid-on-polygon-3](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/grid-on-polygon-3-800-800-1.0-agg-reference.png)

[grid-on-polygon-1-marker-alternating.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/grid-on-polygon-1-marker-alternating.xml)

![grid-on-polygon-1-marker-alternating](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/grid-on-polygon-1-marker-alternating-800-800-1.0-agg-reference.png)
