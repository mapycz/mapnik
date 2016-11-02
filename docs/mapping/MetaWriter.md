# MetaWriter

NOTE: MetaWriters have been [disabled in the Mapnik 2.1.x series](https://github.com/mapnik/mapnik/issues/1240), so if you wish to use them please use Mapnik 2.0.x.

MetaWriters are a way of outputting vector features, encoded as geojson, from Mapnik that exactly match the representations of rendered features. The idea is to allow for highly customizable output of specific features rendered into tiles that should also be made available as vectors to a web browser for interactivity. This support was added by Hermann Kraus (mail: herm @@@ scribus.info) in the GSOC project ["Hit Areas"](GSOC2010_Ideas).

See also the `grid_renderer` which works better for polygons: https://github.com/mapnik/mapnik/wiki/MapnikRenderers

A blog post on the idea is at: http://mapnik.org/news/2010/jul/06/gsoc2010_halfway/.

Slides from State of the Map Europe 2011 are [available](http://http://github.com/mapnik/mapnik/wiki/attachment/wiki/MetaWriter/metawriter-slides-sotmeu.pdf).

See also Herm's demo, which highlights both visible and invisible features: http://r2d2.stefanm.com/mapnik/demo.html

[[/images/metawriter_bbox_around_width_plus_stroke.png]]

The above graphic highlights output (geojson) representing the clickable area of a [[MarkersSymbolizer]] rendered circle. This circle can change size dynamically depending on the width/height of the shape plus the thickness of the stroke and the bbox output by the MetaWriter will be intelligent to all rendering parameters.

## MetaWriter Configuration
The configuration for a MetaWriter is done as a child element of the Map.


```xml
    <Map>
    
      <MetaWriter name="meta1" type="json" file="map_meta.json"/>
    
    <!--..snip..-->
    
    </Map>
```

Parameters are:

 * `name` (required)
  * Assign a name to your metawriter so that it can be referenced from a symbolizer
 * `type` (required)
  * currently 'json' and 'inmem' are the only supported types
 * `file` (json only, required)
  * relative or absolute path to the file-based output (currently json) - in the future could potentially be other kinds of output
  * NOTE: this parameter accepts variable replacement (internally it is a PathExpression) so that for a given render the path can be dynamically constructed (see below for more details)
 * `output-empty` (json only, optional)
  * if true (default) output json files even if no features are intersected, which will write an FeatureCollection with an empty features list
 * `default-output` (optional)
  * this is a comma delimited list of attributes to output for every metawriter attached to a symbolizer
  * example: `default-output="field1,field2"`
  * if not specified, for attributes to be output they must be requested in the symbolizer using the `meta-output` parameter
 * `pixel-coordinates` (relevant to json output format only, default=off) Output in image coordinates rather than long/lat (epsg:4326).


## Caveats

 * If no features are encountered no json file is output (unless you pass the option `output-empty="true"`)
 * output projection is currently hardcoded to WGS84/EPSG:4326
 * precision for geometries is currently fixed at 8

## Attaching writers to symbolizers

To actually trigger a metawriter to output features you need to attach one to a symbolizer.

Parameters that symbolizers accept (the relate to metawriters) are:

  * `meta-writer` - the name of the MetaWriter to use
  * `meta-output` - attributes to write out for this specific symbolizer

For example, this would output geojson features  (bbox's as polygons) for every point rendered by the symbolizer:

```xml
    <PointSymbolizer meta-writer="meta1"/>
```

Example feature output:

```python
    { "type": "Feature",
      "geometry": { "type": "Polygon",
        "coordinates": [ [ [11.07451289, 49.45187982], [11.07481352, 49.45187982], [11.07481352, 49.45218044], [11.07451289, 49.45218044] ] ]},
      "properties": {
    } },
```

This would do the same but also dump out the values for the attributes of 'amenity' and 'name':

```xml
    <PointSymbolizer meta-writer="meta1" meta-output="amenity,name"/>
```

Example feature output:


```python
    { "type": "Feature",
      "geometry": { "type": "Polygon",
        "coordinates": [ [ [11.07804522, 49.45285684], [11.07834585, 49.45285684], [11.07834585, 49.45315747], [11.07804522, 49.45315747] ] ]},
      "properties": {
        "amenity":"pharmacy",
        "name":"Spital Apotheke"
    } },
```

## Dynamically constructing metawriter file path

To dynamically contruct a file path for where to write out meta-features you can use variable replacement like:

### in your xml

```xml
    <MetaWriter name="meta1" type="json" file="[tile_dir]/[z]/[x]/[y].json"/>
```

### in your calling program

```python
    import os
    import mapnik2 as mapnik
    
    m = mapnik.Map(256,256)
    x,y,z = 0,1,1
    tile_dir = '.'
    mapnik.load_map(m,'map.xml') # this 'map.xml' must contain a MetaWriter like above
    m.zoom_all()
    m.set_metawriter_property("x", str(x))
    m.set_metawriter_property("y", str(y))
    m.set_metawriter_property("z", str(z))
    m.set_metawriter_property("tile_dir", tile_dir)
    dir_ = '%s/%s/%s' % (tile_dir,z,x)
    if not os.path.exists(dir_):
        os.makedirs(dir_)
    mapnik.render_to_file(m,'%s/%s/%s/%s.png' % (tile_dir,z,x,y))
```

## Example XML (with PathExpresion)

```xml
    <Map bgcolor="white" srs="+proj=latlong +datum=WGS84">
    
      <MetaWriter name="points" type="json" file="[tile_dir]/[z]/[x]/[y].json"/>
    
      <Style name="points">
        <Rule>
            <Filter>([amenity] neq '') and ([name] neq '')</Filter>
          <PointSymbolizer meta-writer="points" meta-output="amenity,name"/>
        </Rule>
      </Style>
    
      <Layer name="osm" srs="+proj=latlong +datum=WGS84">
        <StyleName>points</StyleName>
        <Datasource>
          <Parameter name="type">osm</Parameter>
          <Parameter name="file">testing.osm</Parameter>
        </Datasource>
      </Layer>
    
    </Map>
```

## Example XML (without PathExpresion)

```xml
    <Map bgcolor="white" srs="+proj=latlong +datum=WGS84">
    
      <MetaWriter name="points" type="json" file="points.json"/>
    
      <MetaWriter name="lines" type="json" file="lines.json"/>
    
      <Style name="points">
        <Rule>
            <Filter>([amenity] neq '') and ([name] neq '')</Filter>
          <PointSymbolizer meta-writer="points" meta-output="amenity,name"/>
        </Rule>
      </Style>
    
      <Style name="lines">
        <Rule>
          <Filter>([highway] neq '') and ([name] neq '')</Filter>
          <LineSymbolizer stroke-width="5" stroke="green" meta-writer="lines" meta-output="highway,name"/>
        </Rule>
      </Style>
    
      <Layer name="osm" srs="+proj=latlong +datum=WGS84">
        <StyleName>lines</StyleName>
        <StyleName>points</StyleName>
        <Datasource>
          <Parameter name="type">osm</Parameter>
          <Parameter name="file">testing.osm</Parameter>
        </Datasource>
      </Layer>
    
    </Map>
```