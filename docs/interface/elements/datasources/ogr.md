# OGR plugin

This plugin supports the [OGR](http://www.gdal.org/ogr/index.html) library in order to read multiple spatial vector formats.


# Installation

Make sure that running _python scons/scons.py DEBUG=y_ shows the following line

    Checking for C library gdal... yes
    Checking for name of ogr library... gdal

To check if the ogr plugin built and was installed correctly, try the usual Python _from mapnik import *_ on a DEBUG=y build, and look for the following debug line

    registered datasource : ogr


# Parameters

| *parameter*       | *value*  | *description* | *default* |
|-------------------|----------|---------------|-----------|
| file                  | string       | file to display | |
| base                  | string       | base path where to search for file parameter | |
| layer                 | string       | name of the layer to display (a single ogr datasource can contain multiple layers) | |
| layer_by_index        | integer      | index of the layer to display, this becomes mandatory if no "layer" parameter is specified | |
| layer_by_sql | string | SQL statement to execute against the OGR-datasource. The result set is used as the layer definition.|
| multiple_geometries   | boolean      | whether to use multiple different objects or a single one when dealing with multi-objects (this is mainly related to how the label are used in the map, one label for a multi-polygon or one label for each polygon of a multi-polygon)| false |
| encoding              | string       | internal file encoding | utf-8 |
| string              | string | optional (replaces *file* parameter) string of literal OGR-datasource data, like GeoJSON |
| extent | string | maximum extent of the layer. should be provided when an extent cannot be automatically determined by OGR |



# Usage

*Note*: The layer names of OGR datasources are returned by Mapnik in the error message when you do not provide the `layer` parameter.

```python
    >>> import mapnik
    >>> mapnik.Ogr(file='test_point_line.gpx')
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
      File "/Library/Python/2.5/site-packages/mapnik/__init__.py", line 295, in Ogr
        return CreateDatasource(keywords)
    RuntimeError: missing <layer> parameter, available layers are:  'waypoints'  'routes'  'tracks'  'route_points'  'track_points'
    >>> mapnik.Ogr(file='test_point_line.gpx',layer='waypoints')
    <mapnik.Datasource object at 0x23f6b0> # works!
```

However the best way to discover the layer names is to use the OGR provided utility called `ogrinfo`. For example running `ogrinfo` on a test GPX files provided with the OGR source code reveals the layer names and geometry types:


```sh
    $ ogrinfo test_point_line.gpx
    Had to open data source read-only.
    INFO: Open of `test_point_line.gpx'
          using driver `GPX' successful.
    1: waypoints (Point)
    2: routes (Line String)
    3: tracks (Multi Line String)
    4: route_points (Point)
    5: track_points (Point)
```

## XML


```xml
        <Layer name="gps_waypoints">
            <StyleName>waypoint_styles</StyleName>
            <Datasource>
                <Parameter name="type">ogr</Parameter>
                <Parameter name="file">test_point_line.gpx</Parameter>
               <Parameter name="layer">waypoints</Parameter>
            </Datasource>
        </Layer>
```

```xml
        <Layer name="states">
            <StyleName>states_shp_styles</StyleName>
            <Datasource>
                <Parameter name="type">ogr</Parameter>
                <Parameter name="base">tests/data</Parameter>
                <Parameter name="file">us_states.shp</Parameter>
                <!-- OGR supports formats with multiple layers and while shapefiles
                only have one, we still need to specify it by name -->
               <Parameter name="layer">us_states</Parameter>
            </Datasource>
        </Layer>
```

```xml
    <Layer name="mssql" srs="+proj=latlong +datum=WGS84">
        <StyleName>mssql_style</StyleName>
        <Datasource>
            <!-- mssql database must contain geometry_columns and spatial_ref_sys metadata tables-->
            <Parameter name="type">ogr</Parameter>
            <Parameter name="string">MSSQL:server=localhost;database=gis;trusted_connection=yes</Parameter>
            <Parameter name="layer_by_sql">SELECT * FROM dbo.planet_osm_line</Parameter>
            <Parameter name="extent">-180,-90,180,89.99</Parameter>
        </Datasource>
    </Layer>
```

## C++

Plugin datasource initialization example code can be found on PluginArchitecture.

A OGR datasource may be created as follows:


```cpp
    {
        parameters p;
        p["type"]="ogr";
        p["file"]="path/to/my/vector/bridges.tab";
        p["layer"]="bridges";
    
        set_datasource(datasource_cache::instance()->create(p));
    
        // Bridges
        Layer lyr("Bridges");
        lyr.add_style("bridges");
        m.addLayer(lyr);
    }
```

## Further References
