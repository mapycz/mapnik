<!-- Name: Legending -->
<!-- Version: 10 -->
<!-- Last-Modified: 2011/10/10 02:01:25 -->
<!-- Author: Petr Dlouhy -->

# Discussion of a Legending Spec for Mapnik
Should be configured with XML or a python class.

#536 tracks progress on this feature.

# Possible Example
[[/images/legend_example.png]]

## XML

```xml
    <Legend>
    <Title breakline=true stroke-width="1" stroke="black">Education</title>
    <SubTitle> Level of Education Among Population Over Age 20</Subtitle>
    <Item>
    <Symbolizer>
    <CssParameter name="stroke">white</CssParameter>
    <CssParameter name="stroke-width">.5</CssParameter>
    <CssParameter name="fill">#93A462</CssParameter>
    <ItemText face_name="DejaVu Sans Bold" size="10" fill="black">Highest</ItemText>
    </Item>
    ....
    </Legend>
```

## Python

# Feature Wishlist

### Layer symbolization using representative feature geometry

Default behavior would will be to use, for example, a filled rectangle to represent polygon geometries (like the example graphic above)

But an optional filter could be applied to direct the legend renderer to use a specific feature geometry (scaled to fit the legend layout) as the symbol

That would look something like:

[[/images/legend_feature_example.png]]

Syntax might look like:


```xml
    <Item>
    <Filter>[id] = 1</Filter>
    <Symbolizer>
    ...
    
    # or, more simplistically choose a random feature
    <Item use_feature="true">
    <Symbolizer>
    ...
```

### Andy's alternative approach

I'd like to approach things slightly differently. We could keep the stylesheet as close as possible to the "real" one, and concentrate the legend-specific alterations to the layer definitions. For example, if there was a WKT layer (#630) and screen coordinates (#631) then you could have something like

```xml
    [... normal style rules ...]
    <Layer name=pubs srs="screen">
      <Datasource type="wkt">
         <Parameter name="geom">POINT(20,20)</Parameter>
         <Parameter name="attributes">{"amenity","pub"}</Parameter>
      </Datasource>
    </Layer>
```

The label text for the legend could then be chosen and placed with other screen+wkt layers to get the desired effect. Although this means creating custom layers, it's likely to be less work than altering the `<style>` sections of a large map definition.

# See also

Lars Ahlzen has written a Python script for TopOSM which creates HTML snippets with images from a Mapnik style file: http://wiki.openstreetmap.org/wiki/TopOSM/Details#Map_legend