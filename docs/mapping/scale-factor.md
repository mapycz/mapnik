# Scale factor

## Designing stylesheets for variable resolution output

A common problem with map rendering is that rendered features like fonts or symbols are commonly either too small or too large depending on the resolution of the device you are targeting. For example, most Mapnik stylesheets are designed with normal, moderate resolution computer displays in mind. For print graphics or for more modern and high resolution displays like on many mobile devices existing stylesheet design can lead to inscrutably small fonts, symbols, and line symbolization.

## Using `scale_factor` to scale up or down rendering sizes

Mapnik since >= 2.0.0 has supported an optional argument for rendering known as `scale_factor`. This is a fractional value that is used as a multiplier throughout the rendering pipeline. It defaults to `1` meaning that all features will be rendered at the exact pixel size specified in the stylesheet. But if you pass a `scale_factor` of `2` then symbolizer properties like line width, line dash-array separation, svg and png icon size, font size, halo size, glyph spacing, and many more will be scaled up by `2`. You can also design your stylesheet with oversized defaults and scale down by passing a `scale_factor` of `.5` or even `.1`. This scaling down approach can have advantages so that things like PNG/JPEG icons are sharp and correctly pixel snapped at their default size and can be scaled down without loosing quality. The opposite of course is not true - while Mapnik as of 2.2 is able to scale bitmap icons up the quality will not be as good.

## Gochas

If you are using `scale_factor` ensure you are using the latest Mapnik version. The Mapnik 2.2.0 release, for example, implemented many subtle fixes to increase the quality of rendering when using a custom `scale_factor` and future releases will surely fix more issues as they become apparent.

## Usage

From python you can pass a custom `scale_factor` as the third, optional argument to `mapnik.render`:

```python
im = mapnik.Image(width,height) 
scale_factor=2 
mapnik.render(map,im,scale_factor) 
im.save('scaled_up_2x.png')
```

The C++ API also supports an optional third argument for `scale_factor`:

```cpp
double scale_factor = 1.0;
mapnik::agg_renderer<mapnik::image_32> ren(map,image,scale_factor);
ren.apply();
```
