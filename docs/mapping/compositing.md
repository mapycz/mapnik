#Compositing

Mapnik has supported various custom compositing operations with raster layers since 0.7.x and with vector layers since 2.1.x.

For some examples of what you can do with this feature see:

 - http://mapbox.com/tilemill/docs/guides/comp-op/
 - http://mapnik.org/news/2012/08/27/stamen-compositing-mapnik-v2.1/
 - http://mapbox.com/blog/announcing-tilemill-0.10.0/
 - http://mapbox.com/blog/tilemill-compositing-operations-preview/

Compositing is supported both at the feature level and the style level when authoring stylesheets.

You can also hook into the compositing operations outside of rendering, in pure python. See the [compositing tests](https://github.com/mapnik/mapnik/blob/master/tests/python_tests/compositing_test.py) from some api examples.

At the time of this writing over 30 different operations are supported. The full list can be see in the code [here](https://github.com/mapnik/mapnik/blob/master/include/mapnik/image_compositing.hpp#L42-79). What each operation does in terms of pixel math is beyond the scope of this documentation. But in general the majority of operations are directly from the SVG spec and will be similar to what you can do in software like Photoshop. The tail end of the [list](https://github.com/mapnik/mapnik/blob/master/include/mapnik/image_compositing.hpp#L42-79) includes some more custom operations like `grain-merge` and `grain-extract` most useful for hillshading and inspired by algorithms in the gimp.

The implementations for the standard operations can be seen in this [agg header](https://github.com/mapnik/mapnik/blob/master/deps/agg/include/agg_pixfmt_rgba.h#L228-1552). More custom operations implemented recently by Mapnik developers are generally found in this [cpp file that extends agg](https://github.com/mapnik/mapnik/blob/master/deps/agg/src/agg_pixfmt_rgba.cpp).

## Feature level compositing

Feature level compositing can be triggered by setting the `comp-op` property on a symbolizer like:

```xml
<PolygonSymbolizer comp-op="multiply" />
```

You can also access this property in python:

```python
>>> import mapnik
>>> sym = mapnik.PolygonSymbolizer()
>>> sym.comp_op
mapnik._mapnik.CompositeOp.src_over
>>> sym.comp_op = mapnik.CompositeOp.multiply
>>> sym.comp_op
mapnik._mapnik.CompositeOp.multiply
```

Feature-level compositing, during rendering, means that for every geometry processed the `multiply` operation will be used to blend the rendered pixels of the polygon against the destination pixels (all data previously rendered on the canvas whether from previous layers or just another polygon from the same style).

Each symbolizer defaults to `comp-op="src-over"` which means that normal blending of the source and destination pixels will occur. Explicitly setting `comp-op="src-over"` or leaving it off will result in the same exact behavior.

If you wish to highlight places of overlap between features in the same style then feature-level compositing may be more useful. If you wish to instead highlight the way different styles overlap (and to perhaps avoid rendering artifacts from overlapping features) then style-level compositing may be preferable (see below).

## Style level compositing

Style level compositing can be triggered by setting the `comp-op` property for an entire style like:

```xml
<Style comp-op="multiply">
  <Rule>
    <PolygonSymbolizer />
    <LineSymbolizer />
  </Rule>
</Style>
```

You can also access this property in python:

```python
>>> sty = mapnik.Style()
>>> sty.comp_op is None # it is not set
True
>>> sty.comp_op = mapnik.CompositeOp.multiply
>>> sty.comp_op
mapnik._mapnik.CompositeOp.multiply
>>> sty = mapnik.Style()
```

Enabling style-level compositing (setting the `comp-op` property) means that an internal, blank (fully alpha) canvas will be created before rendering a given style. The geometries pulled for that style will be rendered as normal (for all symbolizers), but against this temporary canvas instead of against the main canvas. When rendering is finished then this canvas will be blended back into the main canvas using the "multiply" operation.

The default (if no `comp-op` is set on a Style) is to skip the creation of a temporary canvas. So, while setting the `comp-op` on a style to `src-over` will invoke the default blending method, but it will also be triggering the rendering all the entire style to a separate canvas, which can lead to different output - perhaps desirable, perhaps not - just be aware of this.

As of Mapnik 2.1 you can also set an `opacity` property at the style level. This will also trigger the usage of an internal, blank canvas which all features of the style will be rendered against. When the temporary canvas is blended back into the main canvas the `opacity` of the temporary canvas will be set on the fly to allow for very consistent opacity of features (this can avoid artifacts for overlapping features). In this case the `src-over` compositing operation will be used if no other `comp-op` is set.

### Example of "erasing stroke"
This style will erase what was previously drawn on layers below :
```xml
<Style name="eraser" comp-op="dst-out">
    <Rule>
       <LineSymbolizer stroke-width="3" stroke="#FFFFFF" />
    </Rule>
</Style>
```

## Grouped compositing

Several have ask: So I understand that using Style-level compositing I can control the blending behavior between a given style/layer and all the data rendered before it, but can I also group layers and the control the compositing behavior within a group and then also between different groups? The answer currently is NO, at least via the styling interface (XML) in Mapnik. There is no support for grouping layers yet in Mapnik. But, this could be accomplished with a bit of scripting because much of the compositing functionality of Mapnik is exposed in Python. So, say you have a stylesheet with 4 layers: `layer1, layer2, layer3, and layer4`. You want to composite `layer1` with `layer2` using some custom operation (lets say `multiply`), then composite `layer3` with `layer4` with another composite operation (lets say `overlay`) and the finally blend the first two layers with the second two using `soft-light`. You could break out the two groups of layers into two separate maps, render them each to `mapnik.Image` objects and the composite the result like:

```python
import mapnik
group1 = mapnik.Map(256,256)
mapnik.load_map(group1,'group1.xml')
group1.zoom_all()
im = mapnik.Image(256,256)
mapnik.render(group1,im)
group2 = mapnik.Map(256,256)
mapnik.load_map(group1,'group2.xml')
group2.zoom_all()
im2 = mapnik.Image(256,256)
mapnik.render(group2,im2)
im.premultiply()
im2.premultiply()
im.composite(b,mapnik.CompositeOp.soft_light)
im.demultiply()
im.save('final_result.png')
```
