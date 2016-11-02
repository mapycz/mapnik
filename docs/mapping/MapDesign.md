Mapnik offers a base level of functionality which can be used to design great looking maps.

[TileMill](http://mapbox.com/tilemill) is a useful wrapper around Mapnik for the purpose of map design.

### Finding map data

First, you'll need some data. Mapnik [can currently read](http://mapnik.org/faq/) ESRI Shapefiles, TIFF image files, and can query the PostGIS spatial database. It can also read all OGR and GDAL supported vector and raster datasets.

[Shapefiles](http://en.wikipedia.org/wiki/Shapefile) can be downloaded for many locations in the world (for example, data from the [nationalatlas.gov](http://www.nationalatlas.gov/atlasftp.html) or USGS is freely available). Furthermore, Mapnik is one of the key rendering engines used to render the [OpenStreetMap.org](http://openstreetmap.org/) project. While the OpenStreetMap project offers XML dumps of their datasets, it takes some work to configure a rendering pipeline to draw the entire world.

### Making use of Mapnik

There are three major ways of using Mapnik. You can use Mapnik as a library from C++ code, you can write Python scripts, and/or you can write XML configuration files which are then processed by Mapnik. Or, you can use a mix of these! Check out the ExampleCode page for more details.

### Mapnik Data Model

The basic Mapnik object is the Map. Other important objects include _Layers, Filters, Features, Symbolizers,_ and _Geometry_.

Mapnik users typically only deal with Map Layers, Filters, Rules, and Symbolizers. Whereas Filters and Rules serve as predicates that determine _when_ geometric features are displayed, Mapnik Symbolizers take input data and turn them into graphical form, whether as Points, Lines, Polygons, Raster Images, or Textual Labels.

For more information on Symbolizers, Filters, and Rules, see [[SymbologySupport]].

Internally, a mapnik Map object may have multiple Layers, where each Layer should have a reference to a Datasource (mapnik::datasource_ptr). A typical Datasource (for example, a Shapefile Datasource, an in-memory Datasource, or a Raster Datasource) has multiple features.

(To get access to the fundamental Map geometry, one can query a Map Layer's datasource by passing it a mapnik::query, specifying the
axis-aligned bounding box, or "Envelope".)

### Rendering with Mapnik

As mentioned on SymbologySupport and Michal's blog post, [making sense of Mapnik](http://mike.teczno.com/notes/mapnik.html), order matters when using Mapnik to render maps. It uses the [Painter's algorithm](http://en.wikipedia.org/wiki/Painter's_algorithm) to determine Z-ordering, that is, layers are drawn in a specific order, and the "top" layer is drawn last, above all others.
