<!-- Name: RasterColorizer -->
<!-- Version: 4 -->
<!-- Last-Modified: 2011/06/28 01:28:45 -->
<!-- Author: albertov -->
# RasterColorizer

*New in 0.8*

(Note: This document has been copied and translated from https://github.com/BenMoores/mapnik-trunk/wiki/RasterColorizer

The Raster Colorizer allows the palette of rasters to be modified, for example, colorizing a height map.

To use the colorizer, create a style with a raster symbolizer and add a raster colorizer to it.

The DataSource making use of the style must be a single band raster file opened with the [[GDAL]] input plugin. It will not work with other input plugins; and it will not work if more than one band is loaded. It will not work unless you explicitly request the band to be loaded (the default band=-1 does not work even if the input has a single band). It will not work with the PostGIS Raster gdal driver (as of GDAL-0.11).

The colorizer works in the following way:

 * It has an ordered list of ''stop_s that describe how to translate an input value to an output color.
 * A _stop_ has a value, which marks the _stop_ as being applied to input values from its value, up until the next stops value.
 * A _stop_ has a mode, which says how the input value will be converted to a colour.
 * A _stop_ has a color
 * The colorizer also has default color, which input values will be converted to if they don't match any stops.
 * The colorizer also has a default mode, which can be inherited by the stops.
 * The colorizer also has an epsilon value, which is used in the exact mode.


## Modes

The available modes are inherit, discrete, linear, and exact.
Inherit is only valid for stops, and not the default colorizer mode. It means that the stop will inherit the mode of the containing colorizer.

*Discrete* causes all input values from the stops value, up until the next stops value (or forever if this is the last stop) to be translated to the stops color.

*Linear* causes all input values from the stops value, up until the next stops value to be translated to a color which is linearly interpolated between the two stops colors. If there is no next stop, then the discrete mode will be used.

*Exact* causes an input value which matches the stops value to be translated to the stops color. The colorizers epsilon value can be used to make the match a bit fuzzy (in the 'greater than' direction).


# XML

## RasterColorizer element

Required attributes: none

Optional attributes:

 * _default-mode_ This can be either "discrete", "linear" or "exact". If it is not specified then the default is "linear".
 * _default-color_ This can be any color. If it is not specified then the default is "transparent".
 * _epsilon_ This can be any positive floating point value. The default is a very small number (e.g.  1.1920928955078125e-07 )

Optional sub-elements:

 * _stop_ The list of stops ordered by their value attribute.

## stop element

Required attributes:

 * _value_ The value at which the stop begins to be applied

Optional attributes:

 * _color_ The color of the stop. If not specified, the colorizers _default_color_ will be used.
 * _mode_ The mode of the stop. If not specified, "inherit" will be used.


## Example XML

See https://github.com/mapnik/mapnik/blob/master/tests/data/good_maps/raster_symbolizer.xml for more examples.

In this example XML, the following value to color translation is performed:

 * -inf <= x < -1000 white
 * -1000 <= x < -500 blue
 * -500 <= x < 0 red
 * 0 <= x < 5 yellow blending to white
 * 5 <= x < 10 white blending to red
 * 10 <= x < 15 red blending to green
 * 15 <= x < 17 green blending to black
 * 17 == x black
 * 17 <= x < 18 white
 * 18 <= x < 100 green blending to indigo


```xml
<?xml version="1.0" encoding="utf-8"?>
<Map srs="+proj=latlong +datum=WGS84">
<Style name="elevation">
  <Rule>
    <RasterSymbolizer>
      <RasterColorizer default-mode="linear" default-color="white" epsilon="0.001">
        <stop color="blue"        value = "-1000"  />
        <stop color="red"         value = "-500"   mode = "discrete" />
        <stop color="yellow"      value = "0"      />
        <stop                     value = "5"      />
        <stop color="red"         value = "10"     />
        <stop color="green"       value = "15"     />
        <stop color="black"       value = "17"     mode = "exact"    />
        <stop color="indigo"      value = "100"    />
      </RasterColorizer>
    </RasterSymbolizer>
  </Rule>
</Style>

<Layer name="dataraster">
    <StyleName>elevation</StyleName>
    <Datasource>
        <Parameter name="file">../mapnik2/tests/data/raster/dataraster.tif</Parameter>
        <Parameter name="type">gdal</Parameter>
        <Parameter name="band">1</Parameter>
    </Datasource>
</Layer>
</Map>
```

# Python Bindings

Python bindings are available. The objects are as follows:

## RasterColorizer

__mapnik2.RasterColorizer()_

It has the properties _default_color_, _default_mode_, _epsilon_, and _stops_ (read only).

It has the functions _add_stop_, and _get_color_.

## ColorizerStops

This is an array of _ColorizerStop_ objects. It is the type of the _RasterColorizer_ _stops_ property.

## ColorizerStop

It has the properties _color_, _value_, and _mode_.

## ColorizerMode

This is the enumeration of the stop modes. The values are _mapnik2.COLORIZER_LINEAR_, _mapnik2.COLORIZER_DISCRETE_, _mapnik2.COLORIZER_EXACT_, and _mapnik2.COLORIZER_INHERIT_.

## Example Python

```python
import mapnik2
c = mapnik2.RasterColorizer( mapnik2.COLORIZER_DISCRETE , mapnik2.Color(0,0,0,255) )
c.epsilon = 0.001
c.add_stop(-10)
c.add_stop(15.83, mapnik2.COLORIZER_EXACT)
c.add_stop(20, mapnik2.Color("red"))
c.add_stop(30.25, mapnik2.COLORIZER_LINEAR, mapnik2.Color("green"))
c.get_color(23.124)
c.stops[1].color
```
