`This page is outdated (see #3438). Please don't trust it.`

# Mapnik Renderers

Mapnik supports a variety of rendering backends. See [[OutputFormats]] for comparisons of different output formats.

## agg_renderer | Anti-Grain Geometry

The AGG renderer ([Antigrain Geometry](http://antigrain.com)) is the primary renderer in Mapnik.

* AGG 's fast scanline rendering with subpixel *anti-aliasing* is the standout reason for the beauty of Mapnik output.
 * [Anti-Aliasing](http://en.wikipedia.org/wiki/Antialiasing) and [Subpixel Rendering](http://en.wikipedia.org/wiki/Subpixel_rendering) on Wikipedia 
* The AGG renderer's buffer can easily be encoded in a variety of formats. Currently Mapnik supports writing to png and jpeg.
* Version 2.3 of the AGG C++ library is included/embedded within the source tree of Mapnik and compiled automatically during the Scons process.
* The primary developer of AGG, Maxim Shemanarev, passed away in November 2013. Because AGG is open source, we happily maintain our own version with bugfixes.
* Mapnik can also build against a system version of AGG, but this is NOT RECOMMENDED since packaged versions have likely not been updated with critical bug fixes
    
While Mapnik was the first to use AGG rendering for mapping, the AGG renderer is also now an optional rendering engine in the [MapServer](http://mapserver.gis.umn.edu/docs/howto/agg-rendering-specifics) and [MapGuide](http://trac.osgeo.org/mapguide/wiki/MapGuideRfc40) projects.
    
    
## cairo_renderer | Cairographics
  
The [Cairo](http://cairographics.org/) renderer is an auxiliary renderer in Mapnik.

* Cairo was added in r656 due to its similar reputation for high quality graphics output to various formats
 * http://http://github.com/mapnik/mapnik/wiki/log/trunk/src/cairo_renderer.cpp
* Cairo has the '''added advantage''' of supporting both Vector and Raster output.
* Mapnik can render to any [surface](http://www.cairographics.org/manual/cairo-surfaces.html) supported by cairo, either directly or by rendering to a cairo [context](http://www.cairographics.org/manual/cairo-context.html).
 * You can demo the PNG, JPEG, SVG, PDF, and PS formats using the [OSM export tool](http://openstreetmap.org/export/)
* Cairo is optional during Mapnik Scons build process but is enabled automatically if found (using pkg-config).
 * Pkg-config must find libcairo as well as Cairomm(C++ bindings) and Pycairo (python bindings)
 * If Pkg-config is successful you will see the added compiler flags: `-DHAVE_CAIRO -DHAVE_PYCAIRO`


### Python Example Code

Writing to SVG with Mapnik's Cairo renderer:

```python
    import mapnik
    import cairo
    
    mapfile = 'mapfile.xml'
    projection = '+proj=latlong +datum=WGS84'
    
    mapnik_map = mapnik.Map(1000, 500)
    mapnik.load_map(mapnik_map, mapfile)
    bbox = mapnik.Envelope(-180.0,-90.0,180.0,90.0)
    mapnik_map.zoom_to_box(bbox)
    
    # Write to SVG
    surface = cairo.SVGSurface('mapfile.svg', mapnik_map.width, mapnik_map.height)
    mapnik.render(mapnik_map, surface)
    surface.finish()
    
    # Or write to PDF
    surface = cairo.PDFSurface('mapfile.pdf', mapnik_map.width, mapnik_map.height)
    mapnik.render(mapnik_map, surface)
    surface.finish()
```

 * Note: Cairo can also write to PostScript and other image formats
 * Note: 'mapnik.render()' can also render to Cairo Contexts


## svg_renderer

The SVG renderer is written by Carlos López Garcés, started as part of GSOC 2010 and the "better printing project". The idea is that while the Cairo backend offers both PDF and SVG support, we can do better by having a custom implementation to handle things such as layer grouping, re-used of svg/bitmap symbols, and texts on paths. Only the basics are implemented at this point and those needed custom features are still a ways off, but the renderer has much promise. Currently is is not built by default but can be enabled with the build flag `SVG_RENDERER=True`.

The svg_renderer uses some very cool features of boost karma to generate SVG really fast and should be a good example of ways to leverage boost karma more in the future, potentially for other types of innovative vector output.

## grid_renderer

The Grid renderer is designed to output highly optimized feature "hit grids", known as UTFGrids (based on the final encoding format). It does this by leveraging and extending modular parts of the antigrain geometry library to rasterize feature id's into a buffer, then outputs metadata about the relevant feature attributes for the given ids, all enclosed within a compact, highly compressible json file using utf8 encoded feature ids for space efficiency. The grid_renderer was first available in the Mapnik 2.0.0 release.

The UTFGrid spec provides details of the format:

https://github.com/mapbox/utfgrid-spec

See the implementations list for examples:

https://github.com/mapbox/utfgrid-spec/wiki/Implementations

## Further References

 * [OSGEO Discussion of Rendering](http://wiki.osgeo.org/wiki/OSGeo_Cartographic_Library)
 * [GRASS GIS use of Cairo](http://trac.osgeo.org/grass/browser/grass/trunk/lib/cairodriver)
 * [Cairo Vs. AGG Comparison](http://goodythoughts.blogspot.com/2008/03/why-cairo-vs-agg.html)
 * [Blog post on Skia (Chrome Renderer) in context of AGG and Cairo](http://www.gnashdev.org/?q=node/57)
 * [Agg inclusion in Boost thread - see 'A preliminary proposal: AGG project'](http://lists.boost.org/Archives/boost/2002/05/index.php)
 * [Good intro into anti-aliased font issues](http://www.joelonsoftware.com/items/2007/06/12.html)
