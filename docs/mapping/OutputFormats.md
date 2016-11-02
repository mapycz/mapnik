# Mapnik Output Formats

THIS PAGE IS OLD AND DEPRECATED, PLEASE SEE [[Image-IO]] for more up to date details on Mapnik output formats.

Mapnik supports rendering with both AGG and Cairo (see [[MapnikRenderers]] for more detail), and can create maps or map tiles in a variety of formats.

## Tradeoffs

|  *Mapnik Format*  |  *Renderer*  |  *Type*  |  *Visual Quality*  |  *Rendering Speed**  |  *Size**  | *Relevant Code*  |
|-------------------|--------------|----------|--------------------|----------------------|-----------|------------------|
|png, png32             | AGG    | 32-bit | [png](http://mapnik-utils.googlecode.com/svn/example_code/agg_renderer/world_png.png)                                |                   0.12 s                     |      16 KB        | source:trunk/include/mapnik/png_io.hpp |
|png8, png256           | AGG    | 8-bit  |  [png256](http://mapnik-utils.googlecode.com/svn/example_code/agg_renderer/world_png256.png)                   |                   0.12 s                         |        8 KB       | source:trunk/include/mapnik/png_io.hpp |
|jpeg                   | AGG    | ?      |  [jpeg](http://mapnik-utils.googlecode.com/svn/example_code/agg_renderer/world_jpeg.jpg)                              |                   0.12 s                     |        8 KB       | source:trunk/include/mapnik/jpeg_io.hpp |
|ARGB32 (png)           | CAIRO  | 32 bit |  [png24](http://mapnik-utils.googlecode.com/svn/example_code/cairo_renderer/world_FORMAT_RGB24.png) |                   0.24 s                             |        20 KB        | source:trunk/include/mapnik/cairo_renderer.hpp |
|RGB24 (png)            | CAIRO  | 24 bit | [alpha png32](http://mapnik-utils.googlecode.com/svn/example_code/cairo_renderer/world_FORMAT_ARGB32.png) |          0.24 s                             |      20 KB       | source:trunk/include/mapnik/cairo_renderer.hpp |
|svg                    | CAIRO  | N/A    | [svg](http://mapnik-utils.googlecode.com/svn/example_code/cairo_renderer/world.svg)                                       |                    0.28 s                     |    980 KB          | source:trunk/include/mapnik/cairo_renderer.hpp |
|pdf                    | CAIRO  | N/A    | [pdf](http://mapnik-utils.googlecode.com/svn/example_code/cairo_renderer/world.pdf)                                        |                 0.40 s                       |      232 KB         | source:trunk/include/mapnik/cairo_renderer.hpp |
|ps                     | CAIRO  | N/A    | [postscript](http://mapnik-utils.googlecode.com/svn/example_code/cairo_renderer/world.ps)                               |                    0.36 s                     |       1.4 MB       | source:trunk/include/mapnik/cairo_renderer.hpp |

* Rendering speeds and output sizes based on sample 256 X 256 tiles created using the GettingStarted sample data, run using Mapnik SVN Head (r 747), cairo 1.8.0, and pycairo 1.4.12.

## PNG Quantization
Mapnik >= 0.7.1 supports advanced image encoding options using a key=value:key=value type string append to the format name. So, you can provide a format strings like "png","png24", or "png32" (all equivalent) to render to a default 32 bit full color png. But if you pass a paletted png type keyword of either "png8" or "png256" (both equivalent) then the encoder will also accept options to control subtle aspects of encoding like the number of colors, the algorithm, or the zlib compression.

The options include:

 * c=256 - limit number of colors (default 256), works with octree and hextree

 * t=2 - select transparency mode: 0-no alpha, 1-binary alpha(0 or 255), 2-full alpha range, default is 2, works with octree and hextree (NOTE: in Mapnik >= 2.2 `png:t=0` can be used to write rgb images as per [#1559](https://github.com/mapnik/mapnik/issues/1559))

 * m=o - choose quantization method, available options: o-existing octree, h-new hextree with optimizations, default is octree

 * g=2.0 - kind of gamma correction for pixel arithmetic in hextree method, default value 2.0 worked best with most of my test images, value 1.0 means no gamma correction.

 * z=-1 to 9 - level of compression. -1 is default (same as no value passed), 0 is no compression, 1 is BEST_SPEED, and 9 is BEST_COMPRESSION (available first in Mapnik 2.0.0)

 * s=default|filtered|huff|rle - compression stragies for zlib - see zlib docs for more details (available first in Mapnik 2.0.0)

 * e=miniz - enable using experimental [miniz encoder](https://github.com/mapnik/mapnik/issues/1554) (replaces libpng). In some cases this encoder provides better encoding speeds with minor size differences.

So to use new format i.e. in python:
  view.save("test.png",'png256:m=h')
or other example in c++:
  save_to_file(vw, "test.png", "png256:t=1:c=128");

## Hextree details
The m=h (hextree) method should give more smooth images in case of
transparent maps i.e. hybrid like overlayed on some satellite images.
Time and file size increase or decrease depends on content and image
size, but shouldn't be much in common situation. Octree is still the default
for png8 because for most maps without transparent
background or elevation colored/shaded rasters, existing method should
give acceptable and little bit smaller files.

With Hextree, using less than 64-94 colors is not recommended, because returned
palette isn't optimal then. Using 96-128 should give acceptable quality
with some file size reduction, but probably not more then 10%-20%.

For more details see [mapnik-devel email](http://lists.berlios.de/pipermail/mapnik-devel/2010-March/001081.html) and ticket [#477](https://github.com/mapnik/mapnik/issues/477)


## render functions

Most render functions like `save_to_string` also take a third argument which must be a `rgba_palette` object. You can create palettes by passing a buffer with RGBA values (4 bytes), a buffer with RGB values (3 bytes) or an Adobe Photoshop .act file (generated by the "Save for Web" dialog") with exactly 772 bytes to the `rgba_palette` constructors. Note that palette objects are mutable and contain a lookup table that cache all values ever retrieved for faster encoding.

## Tradeoffs
 TODO: discussion of tradeoffs and situations to use which format

### References
 * http://mapserver.org/output/index.html
 * http://trac.osgeo.org/mapserver/ticket/2436
 * http://www.perrygeo.net/wordpress/?p=50
 * http://mapserver.org/output/agg.html