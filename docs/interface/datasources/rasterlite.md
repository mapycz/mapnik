# RasterLite plugin

This plugin supports reading georaster files embedded into sqlite databases using [RasterLite](http://www.gaia-gis.it/spatialite/index.html) of the excellent Spatialite project.

The use of rasterlite allows to use jpeg / wavelets compressed rasters in sqlite files (even 4GB databases !) quite fast than plain geotiff, thanks to the use of the R-Tree and Pyramid features in sqlite.

 * Note: You must use at least the development version [1.1b] (https://www.gaia-gis.it/fossil/librasterlite/index) to compile the input plugin.

For other plugins see: [[PluginArchitecture]]

# Installation

TODO


# Parameters

| *parameter* | *value*  | *description* | *default* |
|-------------|----------|---------------|-----------|
| file            | string       | sqlite database file path  | |
| base            | string       | optional base path where to search for the sqlite database file  | |
| table           | string       | table in database which contains raster data  | |

# Styling

To style a layer use the [[RasterSymbolizer]]

# Usage

## Python

TODO

## XML

```xml
    <!-- NOTE: must be in the same SRS as your map-->
    <Layer name="Raster">
        <StyleName>raster</StyleName>
        <Datasource>
            <Parameter name="type">rasterlite</Parameter>
            <Parameter name="file">/path/to/my/rasterlite/database.sqlite</Parameter>
            <Parameter name="table">my_raster</Parameter>
        </Datasource>
    </Layer>
```

## C++

Plugin datasource initialization example code can be found on [[PluginArchitecture]].

A Raster datasource may be created as follows:

```cpp
    {
        parameters p;
        p["type"]="rasterlite";
        p["file"]="/path/to/my/rasterlite/database.sqlite";
        p["table"]=my_raster;
    
        set_datasource(datasource_cache::instance()->create(p));
    
        Layer lyr("Raster");
        lyr.add_style("raster");
        m.addLayer(lyr);
    }
```

## Further References

* [Rasterlite How-To](http://www.gaia-gis.it/gaia-sins/rasterlite-docs/rasterlite-how-to.pdf)
