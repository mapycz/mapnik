# PolygonPatternSymbolizer

## Examples

[[/images/polygon_pattern.png]]

### Default

` FIXME: Add image `

#### XML

```xml
    <PolygonPatternSymbolizer width="16" height="16" type="png" file="/path/to/icon.png"/>
```

#### Python

```python
    p = PolygonPatternSymbolizer('/path/to/icon.png','png',10,10) # file, type, width, height
```

mapnik2:

```python
    p = PolygonPatternSymbolizer(PathExpression('/path/to/icon.png'))
```

#### C++


```cpp
    ruly_type rule;
    rule.append(polygon_pattern_symbolizer("path/to/icon.png", "png", 20, 20)); // file, type, width, height
```
