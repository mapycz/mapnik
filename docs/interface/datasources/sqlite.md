# SQLite plugin

One such plugin supports the sqlite ([SQLite](http://en.wikipedia.org/wiki/SQLite)). The plugin works with two types of geometry storage methods - raw OGC WKB and ([Spatialite](http://www.gaia-gis.it/spatialite)) geometries.

You can create a pure SQLite + WKB geometry based sqlite db using ogr like:

    ogr2ogr -f SQLite test.sqlite some.shp

You can create a spatialite enabled db also using ogr:

    ogr2ogr -f SQLite test.sqlite some.shp -dsco SPATIALITE=YES

For more details see: http://www.gdal.org/ogr/drv_sqlite.html

The main difference is that a spatialite enabled db will include a spatial index inside the database. A pure SQLite db will not contain a spatial index, but Mapnik can create one on the fly.

# Parameters

| *parameter*       | *value*  | *description* | *default* |
|:------------------|----------|---------------|----------:|
| file                  | string       | sqlite database file path | |
| base                  | string       | optional base path where to search for the sqlite database file | |
| table                 | string       | name of the table to fetch, this can be a sub-query | |
| metadata              | string       | name of the metadata table where the extent and srid of the table are specified | |
| key_field             | string       | name of the id field of the table | OGC_FID | 
| geometry_field        | string       | name of the geometry field, in case you have more than one in a single table | the_geom |
| extent                | string       | maxextent of the geometries | determined by querying the metadata table |
| row_offset            | integer      | number of rows to skip when querying data | 0 |
| row_limit             | integer      | max number of rows to return when querying data, 0 means no limit | 0 |
| wkb_format            | string       | type of WKB in the geometry field blob, this can be "sqlite" or "spatialite" | sqlite |
| use_spatial_index     | boolean      | choose wheter to use the spatial index when fetching data | true |
| multiple_geometries   | boolean      | wheter to use multiple different objects or a single one when dealing with multi-objects (this is mainly related to how the label are used in the map, one label for a multi-polygon or one label for each polygon of a multi-polygon)| false |
| encoding              | string       | internal file encoding | utf-8 |

# Usage

*Note*: 
 * Spatial tables read from sqlite by Mapnik _must_ have a cooresponding entry in `geometry_columns`.
 * Use the `geometry_field` parameter to specify which field to use if you have >1 geometry in the table/query.

## C++

Plugin datasource initialization example code can be found on PluginArchitecture.

A Sqlite datasource may be created as follows:

```cpp
    {
        parameters p;
        p["type"]="sqlite";
        p["file"]=sqlite_spatial_database_file;
        p["table"]="bridges";
        p["geometry_field"]="geom";
        p["extent"]="2309554.99818767,5024797.73763923,2318414.90507308,5040447.94690007";
    
        set_datasource(datasource_cache::instance()->create(p));
    
        // Bridges
        Layer lyr("Bridges");
        lyr.add_style("bridges"); // in style.xml
        m.addLayer(lyr);
    }
```

## Further References
