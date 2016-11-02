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
| max_async_connection  | integer       | max number of PostGIS queries for rendering one map in asynchronous mode. Full doc [here](#postgis-async). ~~Used only when asynchronous_request=true.~~ Default value (1) has no effect. | 1 |

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

## Optimize Rendering with PostGIS

When rendering maps with Mapnik, using a [PostGIS](/wiki:PostGIS/) database as the backend, there is often a large speed gain to be made by employing several optimizations. These optimizations relate to the interaction of PostGIS SQL queries and Mapnik's layers, rules, and filters.

This page will detail some measures you can take to make sure you're not wasting time and memory when rendering a map, or energy maintaining a stylesheet. They are particularly relevant to the rendering of OpenStreetMap data with Mapnik (imported into PostGIS using osm2pgsql), but readers should feel free to edit and extend the instructions with examples from other datasets.

### Keep the SQL resultset as small as possible

#### Basics
First, you have to understand how the Datasource table parameter works for each Layer. What happens is that Mapnik takes what you have defined in your `<Parameter name="table>...</Parameter>`, and slaps a `SELECT AsBinary("way") AS geom,column,names,needed` in front of it, and ending with some SQL to limit results to the BBOX used for the map that's rendering. Mapnik will intelligently limit the columns queried for to what's actually needed in the Styles referenced in the Layer.

Often, when making a new layer and not paying much attention to the Datasource, you may be doing:

```xml
    <Parameter name="table">planet_osm_point</Parameter>
```

This is fine, and will get your data from the database. In fact, it will get *all* data from that table that is within the area that you're rendering. Taking the above explanation, Mapnik expands this to:


```sql
    SELECT AsBinary("way") AS geom
    FROM planet_osm_point 
    WHERE "way" && SetSRID('BOX3D(-39135.75848201022 6222585.598639628,665307.8941941741 6927029.251315812)'::box3d,900913)
```

The SELECT will also add all columns needed to satisfy the styles for this layer. The "WHERE ..." ending will only return rows with an actual geometry (or there'd be nothing to draw for that row anyway) limited to the BBOX given. The coordinates above are just an example.

This type of query is not very efficient. Other than the check for a valid geometry, it will simply fetch each and every row from that table. When the results get to the rules, most of the rows will be filtered out and discarded.


#### Limiting the amount of rows to fetch

Let's say you have a style that's only rendering place names for a given layer. You're not rendering anything else in this style, so why would you fetch records from the database for this layer that have nothing to do with place names? Let's limit the query to just what we need:

```xml
    <!-- the table parameter for a Layer with just place name styles associated with it -->
    <Parameter name="table">(select * from planet_osm_point where place is not null) as foo</Parameter>
```

This is an example of a subquery. Mapnik will still put its own SELECT and WHERE "way" etc. around it, and PostGIS will actually see this query:

```sql
    SELECT AsBinary("way") AS geom from
    (select * from planet_osm_point where place is not null) as foo
    WHERE "way" && SetSRID('BOX3D(-39135.75848201022 6222585.598639628,665307.8941941741 6927029.251315812)'::box3d,900913)
```

Let's say you're filtering for some water features, and only want to render rivers, canals, drains, and streams:

```sql
    (SELECT * from planet_osm_line where waterway in ('river','canal','drain','stream')) as foo
```

If you only want to render road tunnels in a style:

```sql
    (SELECT * from planet_osm_line where highway is not null and tunnel in ('yes','true','1')) as foo
```

#### Limiting the columns fetched from the database

Doing a `SELECT *` will fetch all columns for the records you want. You can further limit this to only the fields you need in your style.

```sql
    (select way,place from planet_osm_point where place is not null) as foo
```

Notice the `way` column. You always need to include this, or else Mapnik will not get the geometry from the database, and nothing can be drawn.

Assume you're also testing for capital cities. You're not testing for `capital` in the SQL, but you will be testing for that in the Mapnik rules. That means the `capital` field needs to be included in the results, by putting it into the `SELECT`:

```sql
    (SELECT way,place,capital from planet_osm_point where place is not null) as foo
```

You may be building some rules that render elements based on the length of a field, let's say for different sized highway shields:

```sql
    (SELECT way,highway,ref,char_length(ref) as length from planet_osm_line where highway is not null and ref is not null) as foo
```

Notice again that you only need to fetch data from the database that will actually render for this style. Assuming that you're rendering highway shields, it makes no sense to fetch highways without a _ref_.

### Move filtering from Mapnik to PostGIS

Often, a Mapnik style will only render a certain element. You would normally use a filter to only draw something for those elements:

```xml
    <Filter>[power] = 'tower'</Filter>
```

used together with:

```sql
    (select * from planet_osm_point) as foo
```

But this means that you a) select all rows from the database from that table and b) Mapnik then needs to discard most of those rows. Same as in the examples above. If your style only contains a single type of data to render, you can omit the Mapnik filtering altogether and do all filtering in SQL:


```sql
    (SELECT way,power from planet_osm_point where power='tower') as foo
```

You may be rendering only tunnels for roads in a particular style:

```sql
    (SELECT way,highway from planet_osm_line where highway is not null and tunnel in ('yes','true','1')) as foo
```

Then you don't need to include a filter to test for tunnel in the Mapnik rules. Usually this means you can simplify things:

```xml
    <Filter>[highway] = 'motorway' and ([tunnel] = 'yes' or [tunnel] = 'true' or [tunnel] = '1')</Filter>
```
    
becomes

```xml    
    <Filter>[highway] = 'motorway'</Filter>
```

### Simplify redundant filter elements

Let's elaborate some more on the last filter example. You may have a style that's testing for `tunnel` but some rules also have to render if an element is *not* a tunnel.

So you'll see a mix of filters in the style:

```xml
    <Filter>[highway] = 'motorway' and ([tunnel] = 'yes' or [tunnel] = 'true' or [tunnel] = '1')</Filter>
```

and another rule with

```xml
    <Filter>[highway] = 'motorway' and not ([tunnel] = 'yes' or [tunnel] = 'true' or [tunnel] = '1')</Filter>
```

Typing all those tunnel tests (actually three tests rolled into one) will get tiresome after a while, and they don't add to the readability of the XML.

SQL to the rescue! Let's have postgresql roll all those choices into 1 simple one to work with in the Mapnik filters:

```sql
    (SELECT way,highway,
    case when tunnel in ('yes','true','1') then 'yes'::text
    else tunnel
    end as tunnel
    from planet_osm_line where highway is not null) as foo
```

The query is split over multiple lines so it's easy to see what's going on. You can also split your own queries into multiple lines in the stylesheet (Mapnik 0.6.0 > | see [#173](https://github.com/mapnik/mapnik/issues/173)), if it helps you to understand long queries.

Now you can replace the two filters with these:

```xml
    <Filter>[highway] = 'motorway' and [tunnel] = 'yes'</Filter>
    
    <Filter>[highway] = 'motorway' and not [tunnel] = 'yes'</Filter>
```

You could even expand the `case when ... end` to also handle the `no` cases, and increase the readability some more.

```sql
    (SELECT way,highway,
    case when tunnel in ('yes','true','1') then 'yes'::text
     else 'no'::text
     end as tunnel
    from planet_osm_line where highway is not null) as foo
```

```xml
    <Filter>[highway] = 'motorway' and [tunnel] = 'yes'</Filter>
    <Filter>[highway] = 'motorway' and [tunnel] = 'no'</Filter>
```

In these examples, the SQL will get more elaborate, but actual filters will be greatly simplified. Since there is only a single datasource query for a layer, and potentially lots of rules and filters to test these results, it stands to reason to do the hard work in a single location, and leave it to the component that does this the best: PostGIS.

### Create indices

Creating indexes can also help fetching the rows faster. To create a GIST index:

```sql
    CREATE INDEX idx_buildings_the_geom ON buildings USING gist(the_geom);
```

Also, if you are filtering or sorting on a specific field, an index on that field can help too:

```sql
    CREATE INDEX idx_buildings_code ON buildings USING btree(code);
```

### Use asynchronous PostGIS datasource
PostGIS datasource can be used asynchronously so SQL querries are processed by the postgres server while mapnik does actual rendering. Fine tuning explained in [[Postgis-async]].

### General Postgresql maintenance

Keep your database optimized. You should have autovacuum turned on. If you have a datasource which is being updated (e.g. OpenStreetMap data) you will need to periodically ``ANALYZE`` and ``REINDEX`` the database. You should run this SQL command from time to time:

```sql
    ANALYZE; REINDEX;
```

Depending on your needs you may want to also ``CLUSTER`` the data periodically.

If there is any active connection Postgresql will wait until it is closed, so if you are running Ogcserver restart Apache to close the connections.

### Use an extent parameter
If an extent parameter is not set, mapnik will perform a query like this...

```sql
SELECT ST_XMin(ext),ST_YMin(ext),ST_XMax(ext),ST_YMax(ext)
FROM (SELECT ST_Extent(geom) as ext from planet_osm_line) as tmp
```

...which requires PostGIS to walk the entire result set of the queried table each time the DataSource is used for the first time in a rendering session.  There are three parameters available for use to avoid this process:

#### extent_from_subquery
E.g.
```xml
<Parameter name="extent_from_subquery">true</Parameter>
```

**Pros:**
* Precise estimate of extent

**Cons:**
* Performance gains only on small result sets

**Prerequisites:**
* table parameter uses a subquery, not just a table name
* extent parameter is not set
* estimate_extent parameter is not set or false

**Example use case:**
* tile server where render requests return small features sets or any render requests with large feature sets are pre-rendered and cached

#### extent
E.g.
```xml
<Parameter name="extent">-20037508,-19929239,20037508,19929239</Parameter>
```

**Pros:**
* No database overhead

**Cons:**
* XML needs to be updated if alterations to source data affects extent
* Less precision -- bad because [I don't know -- something to do with clipping calculations?]

**Prerequisites:**
* coordinates must be provided in appropriate SRS

**Overrides:**
* extent_from_subquery
* estimate_extent

**Example use case:**
* Seldom changing result set with few updates

#### estimate_extent
E.g.
```xml
<Parameter name="estimate_extent">true</Parameter>
```

**Pros:**
* Faster than not setting any extent parameters; significantly for large result sets

**Cons:**
* For PostgreSQL>=8.0.0 statistics are gathered by VACUUM ANALYZE and resulting extent will be about 95% of the real one. -- [PostGIS docs](http://postgis.net/docs/ST_EstimatedExtent.html)

**Overrides**
* extent_from_subquery

**Example use case:**
* [TODO]

## Further References
 [Using Mapnik and PostGIS with OSM](http://wiki.openstreetmap.org/index.php/Mapnik/PostGIS)
