# Mapnik configuration XML

## General

Comments can be placed in the configuration file using the default xml <!-- --> syntax

## Map
The Map object defines the master object of a mapnik configuration XML. It defines map wide parameters and serves as the envelope for Style and Layer definitions.

 * Element: *Map*
 * Element type: Root element

 * Attributes:
  * *background-color*: HTML color code for the background-color of the map (for instance #000000)
    * 'bg-color' before Mapnik2.
    * Opacity is controlled by the last two digits of an 8-digit value. #00000000 means transparent background. The default value is #000000FF.
  * *background-image*: Available in Mapnik2: use an image for the background instead of a color fill.
  * *font-directory*: Available in Mapnik2: pass a directory that contains fonts, which will automatically be registered if they end in ttf, otf, ttc, pfa, pfb, ttc, or dfont.
  * *srs*: Coordinate system in which the map is rendered (for instance '+proj=latlong+datum=WGS84' for a WGS84 Geographic coordinate system)
  * *buffer-size*: Default 0; Good value is usually tile size/2 to help avoid cut labels. This influences envelope used by placement detector ( i.e. 'avoid_edges' parameter). Also set maximum-extent, otherwise you will get problems for bboxes near the borders of your map.
  * *maximum-extent*: Set to maximum extent of (projected) map, ie. in coordinates of result map. For instance "-20037508.34, -20037508.34, 20037508.34, 20037508.34". See also [[BoundsClipping]].
  * *paths-from-xml*: Check if relative paths should be interpreted as relative to/from XML location (default is true)
  * *minimum-version*: Declare the minimum version of mapnik to be used with the stylesheet. Example: minimum-version="0.6.1". Will print a notice if you use an older mapnik version. 

 * Children:
  * *[Style](#style)*
  * *[Layer](#layer)*
  * *FileSource*: See [37f49e2](https://github.com/mapnik/mapnik/commit/37f49e29cce2d334fe9839)
  * *Datasource*: See [Datasource](#datasource) and [37f49e2](https://github.com/mapnik/mapnik/commit/37f49e29cce2d334fe9839)
  * *FontSet*: Defines a fontset for fallback fonts (if a character isn't found in the first font, proceed through the list until it is found)
  * *Include*: The container tag used to wrap all context in files included via XInclude

  * *xmlcomment*: (Ignored by Mapnik)
  * *xmlattr*: (Ignored by Mapnik)


## Style
A Style object  defines the way objects can be rendered. A Mapnik configuration file can have an unlimited number of Style objects. Style objects are referenced by Layer objects in order to actually be rendered.

 * Element: *Style*
 * Element type: Collection of Rules
 * Attributes
  * *name*: Name for this Style object. Needs to be unique per configuration file. A Style is referenced by a Layer through the corresponding StyleName parameter. If a name is ommited, what will happen?
  * *opacity*: Style level opacity: 1 is fully opaque while zero is fully transparent and .5 would be 50% transparent. See [#314](https://github.com/mapnik/mapnik/issues/314)
  * *filter-mode*: Whether *all* rules in the Style should be evaluated (default) or if rendering should stop after the *first* matching rule. See [#706](https://github.com/mapnik/mapnik/issues/706)

 * Children:
  * *[Rule](#rule)*

  * *xmlcomment*: (Ignored by Mapnik)
  * *xmlattr*: (Ignored by Mapnik)


## Layer
 * Element: *Layer*
 * Element type: References a Style (StyleName) and a DataSource

 * Attributes:
  * *name*: The Name of the layer
  * *status*: Default "on"; *on* or *off*, "0" or "1"
  * *clear-label-cache*: Default "off". Setting this to "on" clears the internal placement detector list, causing the items of this layer, and from this layer on, to be rendered without taking previous rendered items into account ('clear collision avoidance list')
  * *cache-features*: Default "off". Setting this to "on" triggers mapnik to attempt to cache features in memory for rendering when (and only when) a layer has multiple styles attached to it. (only available in >mapnik 2 since r2636).
  * *srs*: Default inherits from map.srs; Reference system from the the project [Proj.4](http://trac.osgeo.org/proj/). e.g. +proj=latlong +datum=WGS84
  * *abstract*: Default ""
  * *title*: Default ""
  * *minzoom*: Default 0.0
  * *maxzoom*: Default 1.797693134862316e+308
  * *queryable*: Default "false"

 * Children:
  * *StyleName*: The name of a defined [#Style style]. The style must contain the same string in the attribute *name*.
  * *[Datasource](#datasource)*

## Datasource
 See also the [Python API docs](http://mapnik.org/docs/v2.1.0/api/python/index.html)

 * Element: *Datasource*
 * Element type: References the map data source and parameters.

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


## Rule
 * Element: *Rule*
 * Element type:

 * Attributes
  * *name*
  * *title*

 * Children:
  * *[Filter](elements/filter.md)*
  * *[ElseFilter](elements/else-filter.md)*
  * *[MinScaleDenominator](elements/min-scale-denominator.md)*
  * *[MaxScaleDenominator](elements/max-scale-denominator.md)*
  * *[PointSymbolizer](elements/symbolizers/point.md)* (Similar to MarkersSymbolizer, see [#2115](https://github.com/mapnik/mapnik/issues/2115))
  * *[LineSymbolizer](elements/symbolizers/line.md)*
  * *[LinePatternSymbolizer](elements/symbolizers/line-pattern.md)*
  * *[MarkersSymbolizer](elements/symbolizers/markers.md)*
  * *[ShieldSymbolizer](elements/symbolizers/shield.md)*
  * *[PolygonSymbolizer](elements/symbolizers/polygon.md)*
  * *[PolygonPatternSymbolizer](elements/symbolizers/polygon-pattern.md)*
  * *[TextSymbolizer](elements/symbolizers/text.md)*
  * *[RasterSymbolizer](elements/symbolizers/raster.md)*
  * *[BuildingSymbolizer](elements/symbolizers/building.md)*
  * *[GroupSymbolizer](elements/symbolizers/group.md)*
  * *[DebugSymbolizer](elements/symbolizers/debug.md)*

## Include
 * Element: *Include*
 * Element type: Provides a container for included XML.  Should be used only in included files as the outermost tag.

 * Attributes - None

 * Children:
  * *[Style](#style)*
  * *[Layer](#layer)*
  * *FileSource*:
  * *Datasource*: See [Datasource](#datasource)
  * *FontSet*: Defines a fontset for fallback fonts (if a character isn't found in the first font, proceed through the list until it is found)

