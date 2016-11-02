# GDAL plugin

This plugin supports the [GDAL](http://www.gdal.org/) library in order to read a lot of spatial geo raster data from multiple formats.

# Installation

Make sure that running `python scons/scons.py` shows the following line

    Checking for gdal-config --libs... yes
    Checking for gdal-config --cflags... yes

To check if the gdal plugin built and was installed correctly you can do:

```python
    >>> from mapnik import DatasourceCache as c
    >>> 'gdal' in c.plugin_names()
    True
```

# Parameters

| *parameter* | *value*  | *description* | *default* |
|:------------|----------|---------------|----------:|
| file        | string   | file of the raster to be read | |
| base        | string   | base path where to search for file parameter | |
| band        | integer  | request for a specific raster band index, -1 means all bands. Note that a band read from a single band raster gets interpreted as Grayscale if band=-1 is specified while they retain their original value when explicitly referenced with the "band" parameter. This affects effectiveness of [[RasterColorizer]]  | -1 |
| nodata | double | allow setting nodata value on the fly (will override value if nodata is set in data) | |
| shared | boolean  | Whether to open the dataset in shared mode. Not recommend to enable true unless you can guarantee that one on thread accesses a given file at the same time (unlikely given the way that most tiling software works with mapnik). It is only beneficial therefore if you are rendering multiprocess and single threaded and are working in low memory situation. Internally it directs Mapnik to call `GDALOpenShared` instead of `GDALOpen` which means that GDAL will consult its global cache of datasets. GDAL will try to return a copy of the dataset if access from multiple threads (but this defeats the purpose of sharing) | false |

# Styling

To style a layer from GDAL use the [[RasterSymbolizer]]

# Usage

This plugin in Mapnik >= 0.7.0 supports reading overviews created with http://www.gdal.org/gdaladdo.html

## Python

```python
    style=Style()
    rule=Rule()
    rule.symbols.append(RasterSymbolizer())
    style.rules.append(rule)
    map.append_style('Raster Style',style)
    lyr = Layer('GDAL Layer from TIFF file')
    lyr.datasource = Gdal(base='/path/to/your/data',file='raster.tif')
    lyr.styles.append('Raster Style')
```

## XML

```xml
    <!-- NOTE: must be in the same SRS as your map-->
    <Layer name="GDAL Layer from TIFF file">
    	<StyleName>raster</StyleName>
    	<Datasource>
    		<Parameter name="type">gdal</Parameter>
    		<Parameter name="file">/path/to/your/data/raster.tiff</Parameter>
    	</Datasource>
    </Layer>
```

## C++

Plugin datasource initialization example code can be found on PluginArchitecture.

A GDAL datasource may be created as follows:

```c
    {
        parameters p;
        p["type"]="gdal";
        p["file"]="/path/to/your/data/raster.tiff";
    
        set_datasource(datasource_cache::instance()->create(p));
    
        layer lyr("GDAL Layer from TIFF file");
        lyr.add_style("raster");
        m.add_layer(lyr);
    }
```

## Further References
