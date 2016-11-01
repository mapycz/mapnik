Since Mapnik 2.3.0 the PostGIS plugin can be used asynchronously to potentially reduce the overall map rendering time when stylesheets contain many PostGIS layers.

The functionality was added via pull request [#2010](https://github.com/mapnik/mapnik/pull/2010).

## How it works
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


## When to use it

A good idea:
* You have already applied the [rendering optimizations with PostGIS](OptimizeRenderingWithPostGIS)
* You have at least six PostGIS layers in your stylesheet

Nice to have:
* Rendering time for layers are quite homogeneous
* You already use `cache-features=true` to reduce rendering time in the case of layers with multiple styles
* Your PostGIS database is on another server (so that the extra load on the database from parallel queries can be handled well)

## Problems it will not solve

It will only parallelize queries on different layers. If your layer has multiple styles (like for drawing road casings) then the multiple queries issued for that same layer will not be parallelized. Therefore if you want to speed up this scenario you should instead try using `cache-features=true` which will trigger caching of data on the first pass to avoid issuing the same query twice. Just make sure you have enough RAM to store the query results (this is why `cache-features=true` is not the default).

## When not to use it

* You have less than 3 PostGIS layers - unlikely any benefit.
* You have very heterogenous layers : for example, a huge road layers that takes 8 times longer to render than the other layers. Issuing multiple queries in this case may just slow down the fetch for that single really slow layer and will not benefit your overall rendering times.

## How to use it

### Usage from Python

Instantiate a datasource like:

```python
    lyr = Layer('Geometry from PostGIS')
    lyr.datasource = PostGIS(host='localhost',user='postgres',password='',dbname='your_postgis_database',table='your_table', asynchronous_request=True,max_async_connection=4)
```


## Usage from C++

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

### Usage from XML 
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
### What value to use for max_async_connection and when/for what layers to set it?

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


### Impact on the database server
If `max_async_connection` is set to 4, and the pool of database connection (`max_size`), when rendering one map, the database is likely to receive 20 SQL queries at the same time.

You must ensure the parameter `max_connections` in [postgresql.conf](http://www.postgresql.org/docs/9.3/static/runtime-config-connection.html) can handle at least `max_async_connection` x `max_size`. Be careful when changing `max_connections`, because it might use more memory on the server (see `work_mem` in http://www.postgresql.org/docs/9.3/static/runtime-config-resource.html)

### Tip : how to measure the drawing / waiting-for-database ratio
Monitor your CPU activity while rendering maps in a loop, for example with `htop` under Linux. You must have removed all non-PostGIS layers and set the `asynchronous_request` parameter to **false**. If the activity of the only CPU used is 40%, you can deduce that Mapnik spends 40% of time drawing and 60% waiting for the result of database queries.