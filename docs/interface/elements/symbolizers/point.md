# PointSymbolizer

A PointSymbolizer specifies the rendering of a "graphic symbol" at a point.

If you want to draw a graphic symbol and a text you would better use a ShieldSymbolizer.

## Examples

#### Default

[[/images/point_symbolizer_1.png]]

[[/images/streets2.png]]

#### XML

```xml
    <PointSymbolizer/>
```

#### Python

```python
    sym = PointSymbolizer()
```

#### C++

```cpp
    using mapnik::point_symbolizer;
    point_symbolizer sym;
```

### Image label


    [[/images/point_symbolizer_2.png]]

#### XML

```xml
    <PointSymbolizer file="/tmp/pub.png" width="16" height="16" type="png" />
```

#### Python

```python
    sym = PointSymbolizer("/tmp/pub.png", "png", 16, 16)
    # args are file, type, height, width
    sym.allow_overlap = True
    sym.opacity = .5
```

#### C++

```cpp
    using mapnik::point_symbolizer;
    point_symbolizer sym("/tmp/pub.png","png",16,16);
```

### Allow Overlap

[[/images/allow_overlap=yes.png]]

#### XML

```xml
    <PointSymbolizer allow-overlap="yes" file="/Users/artem/projects/ 
    openstreetmap/mapnik/symbols/station_small.png" type="png" width="5"
    height="5" />
```

### Do Not Allow Overlap

[[/images/allow_overlap=no.png]]

#### XML

```xml
    <PointSymbolizer allow-overlap="no" file="/Users/artem/projects/ 
    openstreetmap/mapnik/symbols/station_small.png" type="png" width="5"
    height="5" />
```

## SVG symbols (trunk)

[[/images/point_symbolizer_svg.png]]

```xml
    <PointSymbolizer file="/Users/artem/Desktop/svg/lion.svg" opacity="1.0" transform="scale(0.2,0.2)" />
```

[[/images/point_symbolizer_svg2.png]]

```xml
    <PointSymbolizer file="/Users/artem/Desktop/svg/lion.svg" opacity="1.0" transform="rotate(45) scale(0.4,0.4)" />
```
