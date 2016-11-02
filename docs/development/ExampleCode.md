<!-- Name: ExampleCode -->
<!-- Version: 11 -->
<!-- Last-Modified: 2009/11/10 17:42:39 -->
<!-- Author: springmeyer -->


# Example Code
The Mapnik source comes with some example code in the [mapnik/demo](https://github.com/mapnik/mapnik/tree/master/demo) directory.

Here are some other sets of Example Code to learn from.

----

## C++ Code

### California
 * *Name*: `cali.cpp`
 * *Description*: A sample program, adapted from demo/c++/rundemo.cpp which demonstrates how one might use the Mapnik C++ API to render the state of California from USGS Shapefile data.
 * *Source*: http://gist.github.com/5009
 * *Tested On*: Mac OS 10.5
 * *Added by:* lwu

### Servers
 * https://github.com/openstreetmap/mod_tile/

### C++ profiling tests
 * http://github.com/jflemer/mapnik-glitz-demo
 * http://bitbucket.org/mishok13/mapnik-perf-testing/

### FreeMap
 * http://svn.openstreetmap.org/applications/rendering/fmapgen/

----

## Python Code

### Hello World
 * *Name*: `world_map_and_styles.py`, `world_map.py`
 * *Description*: A mirror of the GettingStarted tutorial along with sample data.
 * *Source*: 
  * [Pure Python example](http://mapnik-utils.googlecode.com/svn/example_code/hello_world/pure_python/)
  * [Python with XML Mapfile](http://mapnik-utils.googlecode.com/svn/example_code/hello_world/xml_config)
  * [Sample Data for each example](http://mapnik-utils.googlecode.com/svn/example_code/hello_world/data/)
 * *Tested On*: Mac OS 10.5
 * *Added by:* (original mapnik tutorial)

### World Population
 * *Name*: `world_population.py`
 * *Description*: A mirror of the [XmlGettingStarted](/wiki:XMLGettingStarted/) tutorial along with sample data.
 * *Source*: [Python Script, XML Mapfile, and data](http://mapnik-utils.googlecode.com/svn/example_code/world_population)
 * *Tested On*: Mac OS 10.5
 * *Added by:* springmeyer

### Cairo Renderer
[Code samples](MapnikRenderers) for writing to SVG and PDF

### mapnik-utils

The mapnik-utils project includes some example code as well: http://code.google.com/p/mapnik-utils/

### ShieldSymbolizer

An e-mail on [Mapnik ShieldSymbolizer](http://groups.google.com/group/cugos/browse_thread/thread/b62b4890e1933bba). Also describes use of

```python
    $ python 
     >>>import mapnik 
     >>>dir(mapnik) 
     >>>help(mapnik) 
```

and

```sh
    $ pydoc -p 8080 
```

and then open a browser and go to : [http://localhost:8080/mapnik.html](http://localhost:8080/mapnik.html)