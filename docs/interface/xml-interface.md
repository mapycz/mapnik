# Mapnik configuration XML

This is overview of XML interface of Mapnik. For complete reference of the configuration, see [mapnik.org/mapnik-reference/](http://mapnik.org/mapnik-reference/) or [github.com/mapnik/mapnik-reference/](https://github.com/mapnik/mapnik-reference/).

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

References a Style (StyleName) and a DataSource

 * Children:
  * `StyleName`: The name of a defined [#Style style]. The style must contain the same string in the attribute *name*.
  * [`Datasource`](#datasource)

### `Datasource`

References the map data source and parameters.

 * Attributes:
  * *name*: Create a datasource template ([37f49e2](https://github.com/mapnik/mapnik/commit/37f49e29cce2d334fe9839))
  * *base*: Inherit from a datasource template ([37f49e2](https://github.com/mapnik/mapnik/commit/37f49e29cce2d334fe9839))
 * Generic Parameters:
  * type: Specifies the format of the data source
   * Possible values:
     * '''shape'''  :   ESRI shapefile
     * '''postgis'''    :   Postgis table or query
     * '''pgraster'''   :   Postgis table or query (containing or returning raster)
     * '''raster''' :   Tiled or stripped TIFF
     * '''gdal'''   :   GDAL supported raster dataset (not build by default)
     * '''ogr'''          :       OGR supported vector datasource (not build by default)
     * '''osm'''        :   Open Street Map (not build by default)
  * *estimate_extent*: boolean to tell Mapnik to estimate the extent of the layer (true) or not (false)
  * *extent*:       manually enter an extent if estimate_extent is set to false

 * Additional parameters for type *postgis* see: Parameters on the [[PostGIS]] page. 
 * Additional parameters for type *pgraster* see: Parameters on the [[PgRaster]] page. 
 * Additional parameters for type *shape* see [[ShapeFile]]
 * Additional parameters for type *gdal* see [[GDAL]].
 * Additional parameters for type *ogr* see [[OGR]].
 * Additional parameters for type *osm*  see [[OsmPlugin]]


### Rule
 * Element: *Rule*
 * Element type:

 * Attributes
  * `name`
  * `title`

 * Children:
  * [`Filter`](elements/filter.md)
  * [`ElseFilter`](elements/else-filter.md)
  * [`AlsoFilter`](elements/also-filter.md)
  * [`MinScaleDenominator`](elements/scale-denominator.md#minscaledenominator)
  * [`MaxScaleDenominator`](elements/scale-denominator.md#maxscaledenominator)
  * [`PointSymbolizer`](elements/symbolizers/point.md) (Similar to MarkersSymbolizer, see [#2115](https://github.com/mapnik/mapnik/issues/2115))
  * [`LineSymbolizer`](elements/symbolizers/line.md)
  * [`LinePatternSymbolizer`](elements/symbolizers/line-pattern.md)
  * [`MarkersSymbolizer`](elements/symbolizers/markers.md)
  * [`ShieldSymbolizer`](elements/symbolizers/shield.md)
  * [`PolygonSymbolizer`](elements/symbolizers/polygon.md)
  * [`PolygonPatternSymbolizer`](elements/symbolizers/polygon-pattern.md)
  * [`TextSymbolizer`](elements/symbolizers/text.md)
  * [`RasterSymbolizer`](elements/symbolizers/raster.md)
  * [`BuildingSymbolizer`](elements/symbolizers/building.md)
  * [`GroupSymbolizer`](elements/symbolizers/group.md)
  * [`DebugSymbolizer`](elements/symbolizers/debug.md)

### Include
 * Element: *Include*
 * Element type: Provides a container for included XML.  Should be used only in included files as the outermost tag.

 * Attributes - None

 * Children:
  * *[Style](#style)*
  * *[Layer](#layer)*
  * *FileSource*:
  * *Datasource*: See [Datasource](#datasource)
  * *FontSet*: Defines a fontset for fallback fonts (if a character isn't found in the first font, proceed through the list until it is found)

## Examples

A good source of simple examples are visual tests.

## See also

[Python API docs](http://mapnik.org/docs/v2.2.0/api/python/index.html)
