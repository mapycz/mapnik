# Mapnik configuration XML

This is overview of XML interface of Mapnik. For complete reference see [mapnik.org/mapnik-reference](http://mapnik.org/mapnik-reference/) or [github.com/mapnik/mapnik-reference](https://github.com/mapnik/mapnik-reference/).

## General

Comments can be placed in the configuration file using the default xml `<!-- -->` syntax

## Structure

### `Map`
The Map object defines the master object of a mapnik configuration XML. It defines map wide parameters and serves as the envelope for Style and Layer definitions.

 * Children:
  * [`Style`](#style)
  * [`Layer`](#layer)
  * `FileSource`: See [37f49e2](https://github.com/mapnik/mapnik/commit/37f49e29cce2d334fe9839)
  * [`Datasource`](#datasource): See [37f49e2](https://github.com/mapnik/mapnik/commit/37f49e29cce2d334fe9839)
  * [`FontSet`](font-set.md): Defines a fontset for fallback fonts (if a character isn't found in the first font, proceed through the list until it is found)
  * `Include`: The container tag used to wrap all context in files included via XInclude
  * `xmlcomment`: (Ignored by Mapnik)
  * `xmlattr`: (Ignored by Mapnik)

### `Style`

A Style object defines the way objects can be rendered. A Mapnik configuration file can have an unlimited number of Style objects. Style objects are referenced by Layer objects in order to actually be rendered.

 * Children:
  * [`Rule`](#rule)
  * `xmlcomment`: (Ignored by Mapnik)
  * `xmlattr`: (Ignored by Mapnik)

### `Layer`

References a Style (StyleName) and a DataSource.  Each layer can reference 0...N styles.

Order is important -- as in the 'painter model' or [Painter's algorithm](http://en.wikipedia.org/wiki/Painter's_algorithm) to be specific. Layers that are declared first in your XML file, or added programmatically through `Layer::add_style` are drawn first. Layers that are declared or added later are drawn last.

 * Children:
  * `StyleName`: The name of a defined [`Style`](#style). The style must contain the same string in the attribute `name`.
  * [`Datasource`](#datasource)

### `Datasource`

References the map data source and parameters. Mapnik can load data from various data sources. See [Plugin architecture](plugins.md) for more information.

### `Rule`

Each style can have 0...N rules. Rules can have min/max scale denominators, filter expressions and symbolizers.  Rules and filters serve as predicates that determine when geometric features are displayed. Mapnik Symbolizers take input data and turn them into graphical form.


 * Children:
  * [`Filter`](filter.md)
  * [`ElseFilter`](else-filter.md)
  * [`AlsoFilter`](also-filter.md)
  * [`MinScaleDenominator`](scale-denominator.md#minscaledenominator)
  * [`MaxScaleDenominator`](scale-denominator.md#maxscaledenominator)
  * [`PointSymbolizer`](symbolizers/point.md) (Similar to MarkersSymbolizer, see [#2115](https://github.com/mapnik/mapnik/issues/2115))
  * [`LineSymbolizer`](symbolizers/line.md)
  * [`LinePatternSymbolizer`](symbolizers/line-pattern.md)
  * [`MarkersSymbolizer`](symbolizers/markers.md)
  * [`ShieldSymbolizer`](symbolizers/shield.md)
  * [`PolygonSymbolizer`](symbolizers/polygon.md)
  * [`PolygonPatternSymbolizer`](symbolizers/polygon-pattern.md)
  * [`TextSymbolizer`](symbolizers/text.md)
  * [`RasterSymbolizer`](symbolizers/raster.md)
  * [`BuildingSymbolizer`](symbolizers/building.md)
  * [`GroupSymbolizer`](symbolizers/group.md)
  * [`DebugSymbolizer`](symbolizers/debug.md)

### `Include`

Provides a container for included XML. Should be used only in included files as the outermost tag.

## Examples

A good source of simple examples are visual tests: [github.com/mapnik/test-data-visual/tree/master/styles](https://github.com/mapnik/test-data-visual/tree/master/styles)

## See also

[Python API docs](http://mapnik.org/docs/v2.2.0/api/python/index.html)
