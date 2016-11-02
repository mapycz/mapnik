# MarkersSymbolizer

```xml
<MarkersSymbolizer />
```

[[/images/offsets_directions.png]]

The [[MarkersSymbolizer]] should draw blue directional arrows *in the direction of the geometry* (for things like one-way streets).

In case you notice arrows pointing the wrong direction, this means that the segment has been coded in the wrong way.

The ST_reverse function of Postgis can fix this (The problem will then be to identify the geometries that need to be updated).

### SVG markers

*NEW*: Staring from r1793 [[MarkersSymbolizer]] supports [Scalable Vector Graphics (SVG)](http://www.w3.org/TR/SVG/) as input images:

[[/images/markers_symbolizer.png]]


```xml
<Rule>
    <MaxScaleDenominator>10000</MaxScaleDenominator>
    <MarkersSymbolizer spacing="100" file="/Users/artem/Desktop/svg/ladybird.svg" transform="translate(0 -16) scale(2.0)"/>
</Rule>
```

### Dynamic Ellipses
*NEW*: Starting from r2158 MarkersSymbolizer supports width/height/fill/stroke properties to dynamically draw circles (w == h) or ellipses (w != h) when no SVG file is supplied:

[[/images/dynamic_ellipse_markers.png]]


```xml
<MarkersSymbolizer fill="darkorange" opacity=".7" width="20" height="10" stroke="orange" stroke-width="7" stroke-opacity=".2" placement="point" marker-type="ellipse"/>
```

CAVEAT: these properties do not apply to SVG files, and SVG transforms are not supported for modifying ellipses (yet).

OSM currently renders one-way street arrows with Mapnik using several stacked [LineSymbolizers](LineSymbolizer) with varying dash-arrays, but could potentially use the [[MarkersSymbolizer]] in the future:

```xml
<LinePatternSymbolizer file="/home/mapnik/mapnik/symbols/arrow.png" type="png" width="74" height="8" />
```
