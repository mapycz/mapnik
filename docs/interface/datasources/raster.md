# Raster plugin

This plugin supports reading tiff and geotiff files.

Tiled or Stripped tiff files are required and when rasters are small < 200-300 MB this driver is faster that the [[GDAL]] plugin.

If files are larger it is recommended to build GDAL overviews (with gdaladdo command) and instead read with the [[GDAL]] plugin.

**Note: overviews support is available only in Mapnik >= 0.7.0**

A drawback(or advantage!) of this plugin is that it requires manually setting the file bounds.

 * hint: create a GDAL datasource from your tiff in python, call the envelope() method to get the bounds, and use those.

For other plugins see: [[PluginArchitecture]]

# Installation

To check if the raster plugin built and was installed correctly you can do:


```python
>>> from mapnik import DatasourceCache as c
>>> 'raster' in c.plugin_names()
True
```

# Parameters


<table>
<tr>
<td> <strong>parameter</strong> </td>
<td> <strong>value</strong>  </td>
<td> <strong>description</strong> </td>
<td> <strong>default</strong> </td>
</tr>
<tr>
<td> file            </td>
<td> string       </td>
<td> geotiff file path  </td>
</tr>
<tr>
<td> base            </td>
<td> string       </td>
<td> optional base path where to search for the tiff raster file  </td>
</tr>
<tr>
<td> lox             </td>
<td> double       </td>
<td> lower x corner of the image in map coordinates </td>
</tr>
<tr>
<td> loy             </td>
<td> double       </td>
<td> lower y corner of the image in map coordinates </td>
</tr>
<tr>
<td> hix             </td>
<td> double       </td>
<td> upper x corner of the image in map coordinates </td>
</tr>
<tr>
<td> hiy             </td>
<td> double       </td>
<td> upper y corner of the image in map coordinates </td>
</tr>
<tr>
<td> extent          </td>
<td> string       </td>
<td> extent of the image in map coordinates, comma or space separated (at least <lox> <loy> <hix> <hiy> or <extent> should be passed) </td>
</tr>
<tr>
<td> format          </td>
<td> string       </td>
<td> image format to use, currently only tiff is supported </td>
<td> tiff </td>
</tr>
</table>


# Styling

To style a layer use the [[RasterSymbolizer]]

# Usage

## Python

```xml
<!-- NOTE: must be in the same SRS as your map-->
<Layer name="Raster">
    <StyleName>raster</StyleName>
    <Datasource>
        <Parameter name="type">raster</Parameter>
        <Parameter name="file">/path/to/my/raster/file.tiff</Parameter>
        <Parameter name="lox">min_x</Parameter>
        <Parameter name="loy">min_y</Parameter>
        <Parameter name="hix">max_x</Parameter>
        <Parameter name="hiy">max_y</Parameter>
    </Datasource>
</Layer>

from mapnik import Raster, Layer
raster = Raster(base='/home/mapnik/data',file='/path/to/my/raster/file.tiff',lox=minx,loy=min_y,hix=max_x,hiy=max_y)
lyr = Layer('Tiff Layer')
lyr.datasource = raster

```

## C++

Plugin datasource initialization example code can be found on [[PluginArchitecture]].

A Raster datasource may be created as follows:


```cpp
{
    parameters p;
    p["type"]="raster";
    p["file"]="/path/to/my/raster/file.tiff";
    p["lox"]=min_x;
    p["loy"]=min_y;
    p["hix"]=max_x;
    p["hiy"]=max_y;

    set_datasource(datasource_cache::instance()->create(p));

    layer lyr("Raster");
    lyr.add_style("raster");
    m.add_layer(lyr);
}
```
