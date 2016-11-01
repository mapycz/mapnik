# RasterSymbolizer

The RasterSymbolizer is used to render an image from any [GDAL supported format](http://www.gdal.org/formats_list.html) using the [[GDAL]] plugin, from GeoTiff's using the [[Raster]] plugin or from PostGIS using the [[PgRaster]] plugin.

 * Mapnik supports on-the-fly reprojection of raster layers (like it does for vector layers)
  * However, for optimal performance it can be best to first (externally) warp an image to the Spatial Reference System (srs) used in the map (e.g. using the `gdalwarp` command from the gdal-utilities).

 * As of Mapnik 0.6.0 the RasterSymbolizer supports transparency and composition modes.
  * See the original ticket for details: [#259](https://github.com/mapnik/mapnik/issues/259)
  * See also [[Compositing]] for some of the effects.

 * This Symbolizer is often used with DEM (digital elevation model) data containing missing values (for example NASA SRTM DEM data); to achieve nice rendering, the [gdal_fillnodata.py](http://www.gdal.org/gdal_fillnodata.html) tool might be useful.

 * As of Mapnik 0.8 the RasterSymbolizer can be assigned a [[RasterColorizer]] to color a raw data raster according to a palette. This is useful for visualizing scientific data, dynamically changing the color gradient of a DEM, etc...

![](http://1.tiles.ump.waw.pl/ump_tiles/12/2265/1395.png)
![](http://toolserver.org/~cmarqu/hill/12/2265/1395.png)



Processed as described in http://wiki.openstreetmap.org/wiki/Hillshading_using_the_Alpha_Channel_of_an_Image

# Usage

There are three types of raster datasources: *gdal*, *raster* or *pgraster*:

 * The [[GDAL]] plugin is more convenient as it can read the file extents automatically and supports any GDAL-supported type of file
 * The [[Raster]] driver only works with Tiled or Stripped GeoTIFF files and requires manually setting the file bounds, but can be faster.
 * The [[PgRaster]] driver requires manually setting the band number to use as databand.

## XML Layers

See the [[GDAL]], [[Raster]] and [[PgRaster]] plugin pages for more info


## XML Styles

Default (simply renders a copy of the raster)


```xml
    <Style name="My Style">
        <Rule>
            <RasterSymbolizer/>
        </Rule>
    </Style>
```

Using the new 0.6.0 release opacity / merging / scaling options:

```xml

    <Style name="raster">
        <Rule>
            <RasterSymbolizer
                opacity="0.5"
                <!-- scaling="fast" -->
                scaling="bilinear"
                <!-- scaling="bilinear8" -->

                <!-- mode="grain_merge" -->
                <!-- mode="grain_merge2" -->
                <!-- mode="multiply" -->
                mode="multiply2"
                <!-- mode="divide" -->
                <!-- mode="divide2" -->
                <!-- mode="screen" -->
                <!-- mode="hard_light" -->
            >
                <!-- optional <RasterColorizer/> goes here -->
            </RasterSymbolizer>
        </Rule>
    </Style>
```

## Python Styles

```python
    s = Style()
    r=Rule()
    r.symbols.append(RasterSymbolizer())
    s.rules.append(r)
```

## C++

` FIXME `
