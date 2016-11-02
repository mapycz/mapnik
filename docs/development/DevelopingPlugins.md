# Developing Mapnik Plugins

## Understanding the Architecture

Plugins can be used from C++ to read in different kinds of files. Here's an example of C++ code that implicitly uses the _shape_ plugin to read in a ESRI Shapefile:

```c
    std::string mapnik_dir(argv[1]); // assume mapnik home directory, such as "~/src/mapnik" passed in
    std::string plugins_dir(mapnik_dir + "/plugins/input/");
    datasource_cache::instance()->register_datasources(plugins_dir + "shape"); // ESRI SHP support
    datasource_cache::instance()->register_datasources(plugins_dir + "postgis"); // PostGIS integration
```

and used like so,

```c
     {
         parameters p;
         p["type"]="shape";
         p["file"]="../data/statesp020"; // State Boundaries of the United States [SHP]
    
         Layer lyr("Cali");
         lyr.set_datasource(datasource_cache::instance()->create(p)); // Note use of datasource_cache factory method here!
         lyr.add_style("cali"); // style.xml
         lyr.add_style("elsewhere"); // this file
         m.addLayer(lyr);
     }
```

Let's drill-down into what's actually going on. We constructed a [parameter object](https://github.com/mapnik/mapnik/blob/master/include/mapnik/params.hpp) and passed it to a factory method (datasource_cache::instance()->create(p)). It then returned a shared pointer to a datasource object, which was then passed on a new Layer object.

In C++, the parameters object is-a param_map (a std::map from string keys to "value_holder"s, where a value_holder is a [boost::variant](http://www.boost.org/doc/libs/1_36_0/doc/html/variant.html#variant.intro) that can either hold a double or a string. In other words, "parameters p" is a semi-generic parameter hash, then passed to and read by a factory method.

To specify the kind of datasource, note that we specified p's "type" as "shape". Other options might be "osm", "postgis", or "raster".

## Examples

See the [hello world](https://github.com/mapnik/hello-world-input-plugin) input plugin for an example to get started.
