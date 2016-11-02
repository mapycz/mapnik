# ShieldSymbolizer

ShieldSymbolizer supports all options of [[TextSymbolizer]].

### dx, dy
dx, dy (from TextSymbolizer) only move the text, but not the Shield. This behavior will be changed in a future release of mapnik. (See below).

### placement

`placement="line"` only means placement along a line for ShieldSymbolizer, whereas for TextSymbolizer it rotates the text too. Add the `spacing` parameter to get the ShieldSymbolizer to repeat along the line, otherwise `placement="line"` and `placement="point"` will look the same.

### base
Add
`<FileSource name="foo">/home/bar/baz/</FileSource>`
to the beginning of your stylesheet and then use
`<ShieldSymboliser base="foo" name="bridge">`
to refer to /home/bar/baz/bridge.

## Good to know

ShieldSymbolizer can be used to label points.

E.G. If you want to place points on cities and their name above it. If you try to use a TextSymbolizer and a PointSymbolizer separated you will often see points without texts and/or texts without points.

To draw labeled points configure your shield symbolizer with placement = point and custom value for dx/dy to move the text around the point



## Examples

[[/images/120px-Mapnik-highway-motorway.png]]

Setting up a sample shield symbolizer, from the Cascade Users of OpenSource GeoSpatial (CUGOS) list:
http://groups.google.com/group/cugos/browse_thread/thread/b62b4890e1933bba

#### XML
```xml
    <Style name="My Style">
        <Rule>
            <ShieldSymbolizer face-name="DejaVu Sans Bold" size="6" fill="#000000" file="images/shield.svg" width="20" height="20" spacing="100" transform="scale(2.0,2.0)" min-distance="50">[NAME]</ShieldSymbolizer>
        </Rule>
    </Style>
```

#### Python

```python
    shield = ShieldSymbolizer(Expression('[NAME]'),'DejaVu Sans Bold',6,Color('#000000'),PathExpression('images/ushighway_shield_20.png'))
    shield.min_distance = 50
    shield.label_spacing = 100
    shield.displacement = (dx,dy)
```

#### C++

```cpp
    rule_type rule;
    /* Parameters:
          name
          face name
          size
          color
          image
          image type
          width
          height
    */
    shield_symbolizer ss("[NAME]", "DejaVu Sans Bold", 6, color(0, 0, 0), "/path/to/icon.png", "png", 20, 20);
    ss.set_label_placement(mapnik::LINE_PLACEMENT); // Place label along the line
    ss.set_displacement(dx, dy);
    ss.set_label_spacing(min_distance);
    rule.append(ss);
```

## Resources

http://www.routemarkers.com/
http://www.weait.com/content/badges-badges
