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

## Postgis-async

Since Mapnik 2.3.0 the PostGIS plugin can be used asynchronously to potentially reduce the overall map rendering time when stylesheets contain many PostGIS layers.

The functionality was added via pull request [#2010](https://github.com/mapnik/mapnik/pull/2010).

### How it works
Mapnik uses the painter's algorithm to render maps. It means that layers are drawn sequentially. The inner algorithm is:

```
    For each layer
       For each style attached to the layer
         Query the features from the layer
         Wait for the features...
         For each feature in this layer for given style
             For each matching rule draw the feature
```
In this case, the renderer potentially spends a lot of time waiting for PostGIS to perform and return the query results that will provide the feature resultset.

The new `asynchronous_request` parameter in PostGIS plugin aims to parallelize rendering and queries on the database server : while a layer is rendering, SQL queries for further layers are sent ahead.


### When to use it

A good idea:
* You have already applied the [rendering optimizations with PostGIS](OptimizeRenderingWithPostGIS)
* You have at least six PostGIS layers in your stylesheet

Nice to have:
* Rendering time for layers are quite homogeneous
* You already use `cache-features=true` to reduce rendering time in the case of layers with multiple styles
* Your PostGIS database is on another server (so that the extra load on the database from parallel queries can be handled well)

### Problems it will not solve

It will only parallelize queries on different layers. If your layer has multiple styles (like for drawing road casings) then the multiple queries issued for that same layer will not be parallelized. Therefore if you want to speed up this scenario you should instead try using `cache-features=true` which will trigger caching of data on the first pass to avoid issuing the same query twice. Just make sure you have enough RAM to store the query results (this is why `cache-features=true` is not the default).

### When not to use it

* You have less than 3 PostGIS layers - unlikely any benefit.
* You have very heterogenous layers : for example, a huge road layers that takes 8 times longer to render than the other layers. Issuing multiple queries in this case may just slow down the fetch for that single really slow layer and will not benefit your overall rendering times.

### How to use it

#### Usage from Python

Instantiate a datasource like:

```python
    lyr = Layer('Geometry from PostGIS')
    lyr.datasource = PostGIS(host='localhost',user='postgres',password='',dbname='your_postgis_database',table='your_table', asynchronous_request=True,max_async_connection=4)
```


### Usage from C++

A PostGIS asynchronous datasource may be created as follows:

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
        p["asynchronous_request"]=true;
        p["max_async_connection"]=4;
    
        Layer lyr("Roads");
        set_datasource(datasource_cache::instance().create(p));
        lyr.add_style("roads");
        m.addLayer(lyr);
    }
```

#### Usage from XML 
Set the value of `max_async_connection` and `asynchronous_request` in your existing PostGIS datasources:

```xml
<Layer name="countries" status="on" srs="+proj=latlong +datum=WGS84">
      <StyleName>countries_style_label</StyleName>
      <Datasource>
        <Parameter name="type">postgis</Parameter>
        <Parameter name="host">dbhost</Parameter>
        <Parameter name="dbname">admin</Parameter>
        <Parameter name="user">postgres</Parameter>      
        <Parameter name="password"></Parameter>
        <Parameter name="table">world_worldborders</Parameter>
        <Parameter name="max_size">5</Parameter>
        <Parameter name="asynchronous_request">true</Parameter>
        <Parameter name="max_async_connection">4</Parameter>
      </Datasource>
  </Layer>
```
#### What value to use for max_async_connection and when/for what layers to set it?

`max_async_connection` sets the database connection size that can run in parallel for the rendering of one map. Concretely, it means how many layers to load features for ahead of rendering. If you want to benefit from parallelization, you must ensure that the heaviest layer to draw does not wait for the geographic features, ie the PostGIS query must have been launched early enough.

Let's consider the number of geographical features for 7 layers :

1. countries ->  500
1. urban areas -> 550
1. parcs -> 620
1. commercial areas -> 580
1. lakes -> 570
1. roads -> 2500
1. cities -> 300

In this example we assume database query time and drawing time are equal and proportional to the number of features in the layer (1).

The largest layer is *roads*; it is 4 time larger than the others. Hence we should launch the query to get the features for roads before the drawing of layer *urban areas*, so that the query is finished when the drawing of roads is about to start. So `max_async_connection` can be set to 4.


Anyway, if you have no idea at all, **4** is a good start.


(1) If you use two identical servers, and you have optimized layers according to [rendering optimizations with PostGIS](OptimizeRenderingWithPostGIS), you can assume queries and drawing are about the same time, except for drawing labels that are slower than queries.


#### Impact on the database server
If `max_async_connection` is set to 4, and the pool of database connection (`max_size`), when rendering one map, the database is likely to receive 20 SQL queries at the same time.

You must ensure the parameter `max_connections` in [postgresql.conf](http://www.postgresql.org/docs/9.3/static/runtime-config-connection.html) can handle at least `max_async_connection` x `max_size`. Be careful when changing `max_connections`, because it might use more memory on the server (see `work_mem` in http://www.postgresql.org/docs/9.3/static/runtime-config-resource.html)

#### Tip : how to measure the drawing / waiting-for-database ratio
Monitor your CPU activity while rendering maps in a loop, for example with `htop` under Linux. You must have removed all non-PostGIS layers and set the `asynchronous_request` parameter to **false**. If the activity of the only CPU used is 40%, you can deduce that Mapnik spends 40% of time drawing and 60% waiting for the result of database queries.

## Further References
 [Using Mapnik and PostGIS with OSM](http://wiki.openstreetmap.org/index.php/Mapnik/PostGIS)
