# LinePatternSymbolizer

*Note* that the line direction matters!

## Examples
A natural=cliff tag on an OpenStreetMap tile, rendered with LinePatternSymbolizer.

![Showing a cliff on openstreetmap.org](http://a.tile.openstreetmap.org/18/141423/87855.png)

### Default


#### XML

```xml
    <LinePatternSymbolizer width="16" height="16" type="png" file="/path/to/icon.png"/>
```


#### Python


```python
    l = LinePatternSymbolizer('/path/to/icon.png','png',10,10) # file, type, width, height
```

#### C++

```cpp
    rule_type rule;
    rule.append(line_pattern_symbolizer("/path/to/icon.png", "png", 20, 20)); // file, type, width, height
```
