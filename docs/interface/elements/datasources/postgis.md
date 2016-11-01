# PostGIS plugin

This plugin supports [PostGIS](http://en.wikipedia.org/wiki/PostGIS), a spatial extension to the popular PostgreSQL database.

See also a performance tuning page: [[OptimizeRenderingWithPostGIS]]

## Parameters

| *parameter*       | *value*  | *description* | *default* |
|:------------------|----------|---------------|----------:|
| host                  | string       | name of the postgres host | |
| port                  | integer      | name of the postgres port | |
| dbname                | string       | name of the database | |
| user                  | string       | username to use for connecting | |
| password              | string       | user password to use for connecting | |
| table                 | string       | name of the table to fetch, this can be a sub-query;  subquery has to use syntax of:  '( ) as subquery'. | |
| geometry_field        | string       | name of the geometry field, in case you have more than one in a single table. This field and the SRID will be deduced from the query in most cases, but may need to be manually specified in some cases.| |
| geometry_table        | string       | name of the table containing the returned geometry; for determining RIDs with subselects | |
| srid                  | integer      | srid of the table, if this is > 0 then fetching data will avoid an extra database query for knowing the srid of the table | 0 |
| extent                | string       | maxextent of the geometries | determined by querying the metadata for the table |
| extent_from_subquery  | boolean      | evaluate the extent of the subquery, this might be a performance issue | false |
| connect_timeout       | integer      | timeout is seconds for the connection to take place | 4 |
| persist_connection    | boolean      | choose whether to share the same connection for subsequent queries | true |
| row_limit             | integer      | max number of rows to return when querying data, 0 means no limit | 0 |
| cursor_size           | integer      | if this is > 0 then server cursor will be used, and will prefetch this number of features | 0 |
| initial_size          | integer      | initial size of the stateless connection pool | 1 |
| max_size              | integer      | max size of the stateless connection pool | 10 |
| multiple_geometries   | boolean      | whether to use multiple different objects or a single one when dealing with multi-objects (this is mainly related to how the label are used in the map, one label for a multi-polygon or one label for each polygon of a multi-polygon)| false |
| encoding              | string       | internal file encoding | utf-8 |
| simplify_geometries   | boolean      | whether to automatically [reduce input vertices](http://blog.cartodb.com/post/20163722809/speeding-up-tiles-rendering). Only effective when output projection matches (or is similar to) input projection. Available from version 2.1.x up. | false |
| ~~asynchronous_request~~  | boolean       | ~~Postgres queries are sent asynchronously : while rendering a layer, queries for further layers will run in parallel in the remote server.  Available from version 2.3.x up.~~ | *DEPRECATED* (automatically set internally based on `max_async_connection > 1` condition) |
| max_async_connection  | integer       | max number of PostGIS queries for rendering one map in asynchronous mode. Full doc [here](Postgis-async). ~~Used only when asynchronous_request=true.~~ Default value (1) has no effect. | 1 |

## Usage

*Note*: 

 * Spatial tables read from PostGIS by Mapnik should ideally have a corresponding entry in `geometry_columns`.
 * Use the `geometry_field` parameter to specify which field to use if you have >1 geometry in the table/query or if your table does not have a `geometry_columns` entry.

## Advanced Usage

The PostGIS plugin supports several special tokens. You can use them in subqueries and Mapnik will replace them at render time.

### bbox token

Under normal circumstances, if you pass to Mapnik `table=mytable` then, when `mytable` is ultimately queried, Mapnik will form up a query like:

```
ST_AsBinary("geom") AS geom,"name","gid" FROM mytable WHERE "geom" && ST_SetSRID('BOX3D(<map bbox>)'::box3d, 3857)
```

Using the token !bbox! allows you to write a subquery and leverage the spatial filter in a custom way. So, if you wished to override the `geom &&` and do `ST_Intersects` instead then you could do (in XML):

```xml
<Parameter name="table">(Select * from mytable where ST_Intersects(geom,!bbox!)) as mysubquery</Parameter>
```

### other tokens

Other tokens that can be used include:

  * !scale_denominator! (Mapnik >= 0.7.0)
  * !pixel_width! (Mapnik >= 2.1.0) -- width of a pixel in geographical units
  * !pixel_height! (Mapnik >= 2.1.0) -- height of a pixel in geographical units

## Usage from Python

Instantiate a datasource like:

```python
    lyr = Layer('Geometry from PostGIS')
    lyr.datasource = PostGIS(host='localhost',user='postgres',password='',dbname='your_postgis_database',table='your_table')
```

If you want to do complex queries you can nest subselects in the `table` argument:

```python
    lyr = Layer('Buffered Geometry from PostGIS')
    BUFFERED_TABLE = '(select ST_Buffer(geometry, 5) as geometry from %s) polygon' % ('your_postgis_table')
    lyr.datasource = PostGIS(host='localhost',user='postgres',password='',dbname='your_postgis_database',table=BUFFERED_TABLE)

If you want to add something after the query (for example ORDER BY) you must use !bbox! dynamic map variable:

    lyr = Layer('Order by st_length from PostGIS')
    BUFFERED_TABLE = 'table_line where way && !bbox! ORDER BY st_LENGTH(way) DESC'
    lyr.datasource = PostGIS(host='localhost',user='postgres',password='',dbname='your_postgis_database',table=BUFFERED_TABLE, srid='your_srid', geometry_field='way', extent='your_extent')
```

 * *Note*: because mapnik depends on the `geometry_columns` entry be careful not to use sub-selects that change the geometry type.
 * Further references: See Artem's email on [using the PostGIS from Python](https://lists.berlios.de/pipermail/mapnik-users/2007-June/000300.html)
 * Example code at the Mapnik-utils project: http://mapnik-utils.googlecode.com/svn/example_code/postgis/postgis_geometry.py

## Usage from XML

If you are using XML mapfiles to style your data, then using a PostGIS datasource (with a sub-select in this case) looks like:

 * *Note*: if you use a sub-select that changes the extents of your features, make sure to use `estimate_extent=false` otherwise Mapnik will return no features. Otherwise you don't need to use the `estimate_extent` or `extent` parameters at all.

```xml
    <Layer name="countries" status="on" srs="+proj=latlong +datum=WGS84">
        <StyleName>countries_style_label</StyleName>
        <Datasource>
          <Parameter name="type">postgis</Parameter>
          <Parameter name="host">localhost</Parameter>
          <Parameter name="dbname">geodjango_geographic_admin</Parameter>
          <Parameter name="user">postgres</Parameter>      
          <Parameter name="password"></Parameter>
          <Parameter name="table">(select ST_Buffer(ST_Centroid(geometry),2) as geometry, name  from world_worldborders) as world</Parameter>
          <Parameter name="estimate_extent">false</Parameter>
          <Parameter name="extent">-180,-90,180,89.99</Parameter>
        </Datasource>
    </Layer>
```

*Note*: If you use a custom projection, you might need to change the extent parameters to the area for which the projection is defined. For example, the Dutch grid (EPSG:28992) is only defined around the Netherlands. It does not make sense to try to project South America onto it. You need to change the extent parameter to something like this:

```xml
    <Parameter name="extent">3.09582088671,50.6680811311,7.41350097346,53.6310799196</Parameter>
```

If you don't do this, you might not see data from this data source at all, even if it does not contain data outside of the valid region. Also note that you always specify the extents in the coordinates of the source system.

## Usage from C++

Plugin datasource initialization example code can be found on [[PluginArchitecture]].

A PostGIS datasource may be created as follows:

```cpp
#include <mapnik/version.hpp>
#include <mapnik/datasource_cache.hpp>

    {
        parameters p;
        p["type"]="postgis";
        p["host"]=database_hostname;
        p["port"]="5432";
        p["dbname"]="gis";
        p["user"]=your_username;
        p["password"]="";
    
        Layer lyr("Roads");
#if MAPNIK_VERSION >= 200200
        set_datasource(datasource_cache::instance().create(p));
#else
        set_datasource(datasource_cache::instance()->create(p));
#endif
        lyr.add_style("roads");
        m.addLayer(lyr);
    }
```

For other PostGIS parameters, see [the postgis_datasource constructor in postgis_datasource.cpp#L48](https://github.com/mapnik/mapnik/blob/master/plugins/input/postgis/postgis_datasource.cpp#L48)

TODO -- more PostGIS query usage

## Further References
 [Using Mapnik and PostGIS with OSM](http://wiki.openstreetmap.org/index.php/Mapnik/PostGIS)
