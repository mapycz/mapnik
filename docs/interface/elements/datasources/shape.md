# Shapefile Plugin

This plugin supports reading shapefiles. They can also be read using the [[OGR]] Plugin, but the *shape* plugin is better tested and more mature.

The shapefile plugin will run fastest if you build indexes for your shapefiles using the 'shapeindex' command line tool installed when you build Mapnik.

For other plugins see: [[PluginArchitecture]]

# Installation

To check if the raster plugin built and was installed correctly you can do:

```python
    >>> from mapnik import DatasourceCache as c
    >>> 'shape' in c.plugin_names()
    True
```

## Parameters

| *parameter*       | *value*  | *description* | *default* |
|:------------------|----------|---------------|----------:|
| encoding             | string       | Encoding Used |  utf-8. ESRI Shapefiles are usually 'latin1' |
| file              | string | name of shapefile (without extension) | | 


# Styling

To style a layer, use any of the Symbolizers like Point, Polygon, or Line, depending on the geometry type.

# Usage

## Python

## C++

Plugin datasource initialization example code can be found on PluginArchitecture.

A Shapefile datasource may be created as follows:

```cpp
    {
        parameters p;
        p["type"]="shape";
        p["file"]="path/to/my/shapefile.shp";
    
        // Bridges
        Layer lyr("Vector");
        lyr.set_datasource(datasource_cache::instance()->create(p));
        lyr.add_style("vector");
        m.addLayer(lyr);
    }
```

## XML

```xml
    <Layer name="vector" srs="+init=epsg:4236">
            <StyleName>polygon</StyleName>
            <Datasource>
                    <Parameter name="type">shape</Parameter>
                    <!-- you can also point to your shapefile without the 'shp' extention -->
                    <Parameter name="file">/path/to/your/shapefile.shp</Parameter>
            </Datasource>
    </Layer>
```

## Further References
