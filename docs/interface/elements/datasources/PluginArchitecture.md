## Overview

Mapnik supports a Plugin architecture read access to a variety of formats.

Some plugins have been written in C++ and require no external dependencies, while others require access to a supporting C or C++ library (like libpq for postgis/postgresql and libgdal for gdal/org support).

Interested in developing a new plugin see: [[DevelopingPlugins]]

All current plugins are either located in [plugins/input](https://github.com/mapnik/mapnik/tree/master/plugins/input/) in the source tree, or in the [mapnik/non-core-plugins](https://github.com/mapnik/non-core-plugins) repository.

| **Name**| **Availability**|**Status**|**Build by Default**|**Notes**|
|:-------:|:---------------:|:--------:|:------------------:|:-------:|
| *[postgis](PostGIS)*| 0.2.0| Stable| yes| support for accessing geospatial data through [[PostGIS]], the spatial database extension for [PostgreSQL](http://en.wikipedia.org/wiki/PostgreSQL)|
| *[raster](Raster)*| 0.2.0| Stable| yes|  stripped or tiled raster TIFF image dataset support|
| *[shape](ShapeFile)*| 0.2.0| Stable| yes|  vector SHP ([ESRI Shapefile](http://en.wikipedia.org/wiki/Shapefile)) parsing|
| *[gdal](GDAL/)*| 0.5.0| Beta| no|  support for [GDAL](http://en.wikipedia.org/wiki/GDAL) datasets|
| *[ogr](OGR)* | 0.6.0| Beta| no|  support for [OGR](http://en.wikipedia.org/wiki/GDAL) datasets|
| *[osm](OsmPlugin)*| 0.6.0| Beta| no|  support for reading OpenStreetMap (OSM) xml datasets|
| *[sqlite](SQLite)*| 0.6.0| Beta| no|  support for [SQLite](http://en.wikipedia.org/wiki/SQLite) / [Spatialite](http://www.gaia-gis.it/spatialite) sql vector format|
| *[occi](OCCI)*| 0.6.0| Beta| no|  support for oracle spatial 10g/11g (versions 10.2.0.x and 11.2.0.x) ([Oracle Spatial](http://en.wikipedia.org/wiki/Oracle_Spatial)) sql vector format|
| *[kismet](Kismet)*| 2.1?| Alpha| no|  support for [Kismet](http://www.kismetwireless.net/) GPS; shows little WLAN nodes on the map| 
| *[rasterlite](Rasterlite)*| 2.1?| Experimental| no|  support for [Rasterlite](http://www.gaia-gis.it/spatialite) sqlite raster with wavelet compression| 
| *[geos](GEOS)*| 2.1?| Experimental| no|  support for inline WKT geometries using [GEOS](http://trac.osgeo.org/geos/) library| 
| *[python](Python Plugin)* | 2.1| Experimental| yes| support for generating features dynamically using the Python programming language|
| *[geojson](GeoJSON Plugin)*| 2.1| Experimental| no| support for reading features from [GeoJSON](http://geojson.org/) data|
| *[csv](CSV Plugin)*| 2.1| *unknown*| no| support for reading lon/lat, GeoJSON, or WKT from [CSV](https://www.wikiwand.com/en/Comma-separated_values) data|
| *[pgraster](PgRaster)*| 3.0| Stable | yes | support for loading raster data from PostGIS|
| *[topojson](TopoJSON Plugin)*| 3.0| *unknown*| no| support for loading features from [TopoJSON](https://github.com/mbostock/topojson/wiki) data|

*Note*: When compiling Mapnik from source only the PostGIS, Raster, and Shape plugins will be compiled by default.

To request additional plugins to be compiled and installed make sure you have the required dependencies and then specify the list of plugins for SCons to build:


```sh
    # attempt to build all plugins possible
    $ python scons/scons.py INPUT_PLUGINS='all'
    # build just the postgis plugin:
    $ python scons/scons.py INPUT_PLUGINS='postgis'
    # to compile the plugins statically with mapnik library (https://github.com/mapnik/mapnik/tree/static-plugins)
    $ python scons/scons.py INPUT_PLUGINS='all' PLUGIN_LINKING='static' 
```

## Querying plugins

If you compiled Mapnik with DEBUG=True, then the list of plugins registered by the python bindings will during initial import:

```python
    Python 2.5.1 (r251:54863, Jan 17 2008, 19:35:17) 
    >>> import mapnik
    registered datasource : gdal
    registered datasource : raster
    registered datasource : shape
```

Otherwise, get a listing of available plugins with this command:

```sh
    $ python -c "from mapnik import DatasourceCache as c; print ','.join(c.plugin_names())"
    These are the registered datasource plugins that Mapnik's python binding currently knows about.
```