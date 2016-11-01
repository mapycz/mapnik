# OSM plugin

This plugin allows for the direct reading of data from the [OpenStreetMap XML format](http://wiki.openstreetmap.org/wiki/.osm).

*NOTE*: the primary way that Mapnik is used to render OpenStreetMap data is to import an [extract](http://wiki.openstreetmap.org/wiki/Planet.osm) into Postgres using the [osm2pgsql](http://wiki.openstreetmap.org/wiki/Osm2pgsql) tool and then read it using [[PostGIS]] plugin.


# Dependencies

You need libxml2 installed on your system for parsing the XML.


# Parameters

| *parameter* | *value*  | *description* | *default* |
|-------------|----------|---------------|-----------|
| file            | string       | the OSM file to load | |
| bbox            | string       | the bounding box to load from the URL of an OSM data source (see below). | |
| parser          | string       | the XML parser: currently, this must have a value of "libxml2" as libxml2 is the only parser currently supported | libxml2 |
| filter_factor   | double       | filter to use when querying for raster data | 0.0 |


# Usage

## How to specify an OSM layer in your Mapnik XML file

Your layer's Datasource must have a "type" parameter with a value of "osm", in a similar way that the shapefile plugin needs a "type" parameter of "shape".
For example:


```xml
<Layer name="roads" status="on" srs="+proj=latlong +datum=WGS84">
    <StyleName>residential</StyleName>
    <StyleName>unclassified</StyleName>
    <StyleName>secondary</StyleName>
    <StyleName>primary</StyleName>
    <StyleName>motorway</StyleName>
    <Datasource>
      <Parameter name="type">osm</Parameter>
      <Parameter name="file">test2.osm</Parameter>
    </Datasource>
</Layer>
```

## Styling the output

Styling the output is done in the same way as for other data sources, with tests for different tags done in the `<Filter>` tag. For example this rule will match OSM ways where the 'highway' tag is equal to 'path' and the 'foot' tag is equal to 'designated':

```xml
<Rule>
    <Filter>[highway] = 'path' and [foot] = 'designated'</Filter>
    <LineSymbolizer>
        <CssParameter name="stroke">#fff</CssParameter>
        <CssParameter name="stroke-width">6</CssParameter>
        <CssParameter name="stroke-linejoin">round</CssParameter>
        <CssParameter name="stroke-linecap">round</CssParameter>
        <CssParameter name="stroke-opacity">0.4</CssParameter>
    </LineSymbolizer>
    <LineSymbolizer>
        <CssParameter name="stroke">red</CssParameter>
        <CssParameter name="stroke-width">2.0</CssParameter>
        <CssParameter name="stroke-dasharray">1,4</CssParameter>
        <CssParameter name="stroke-linejoin">round</CssParameter>
        <CssParameter name="stroke-linecap">round</CssParameter>
    </LineSymbolizer>
</Rule>
```

## Lines or polygons?

Polygon support is not yet very sophisticated. A few pre-defined tag/value combinations are assumed to be polygons; all others are assumed to be linear ways.
Currently these tag/values are assumed to be polygons:

- natural=wood

- natural=water

- natural=heath

- natural=marsh

- military=danger_area

- landuse=forest

- landuse=industrial.

These are defined in the polygon_types class in the source file osm.h, so if you want to add others for your own use, that's the place to go.
