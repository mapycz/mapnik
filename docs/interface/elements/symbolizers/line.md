# LineSymbolizer

A LineSymbolizer is used to render a "stroke" along a linear geometry.

## Examples

### Default
```xml
<LineSymbolizer />
```
#### XML
```xml
<LineSymbolizer stroke="#0000ff" stroke-width="4" />
```
#### Python

```python
l = LineSymbolizer(Color('green'),0.1)
```
To work directly with the stroke object:

```python
l = LineSymbolizer()
s = Stroke(Color('green'),0.1)
s.add_dash(.1,.1)
s.opacity = .5
l.stroke = s
```
Fetch all the possible methods like:

```python
>>> from mapnik import LineSymbolizer
>>> dir(LineSymbolizer().stroke)
```

#### C++

```cpp
rule_type rule;
stroke ls;         // This is the line symbolizer
ls.set_color(color(255, 255, 255));
ls.set_width(4);   // width of the line in pixels
ls.set_line_join(mapnik::ROUND_JOIN);
ls.set_line_cap(mapnik::ROUND_CAP);
ls.add_dash(2.5, 1.0);
ls.set_opacity(0.5);
rule.append(ls);
```
