As one might guess from [Wikipedia's entry on GIS](http://en.wikipedia.org/wiki/Geographic_information_system), geographic information systems have a long and rich history.

In this brief note, we will sketch out some basic GIS and GIS-related concepts that can be useful in making use of Mapnik!


----

## What is GIS?

First, we should point out that most would not consider Mapnik a "true geographic information system" since it is mainly a [rendering library](http://en.wikipedia.org/wiki/Rendering_(computer_graphics)) rather than a complete information system that may provide tools for rich analysis or data management. That said, Mapnik builds upon existing work on GIS and GIS-related technologies, such as [proj.4](http://trac.osgeo.org/proj/), [PostGIS](http://en.wikipedia.org/wiki/PostGIS), and various raster/vector data formats.

Instead, some might say that Mapnik may be more easily applicable to endeavors in the field of [Neogeography](http://en.wikipedia.org/wiki/Neogeography).

Terminology aside, as you start using or contributing to Mapnik, you'll notice that Mapnik gives you many options when it comes to choosing your _input datasources_ and _geospatial projections_.

## Data formats

The two broad categories of input datasources are raster graphics and vector graphics. Developers and dedicated users can dive into Mapnik's PluginArchitecture to get an idea of what kinds of raster and vector input formats Mapnik supports.

TODO -- paras on choosing input data sources, whether here or in the MapDesign page.

## Geospatial Projections

As you start writing C++ or Python code, or tweaking existing XML files, you'll notice that Mapnik allows you to specify geospatial map projections like so:


    <Map srs="+proj=latlong +datum=WGS84 +k=1.0 +units=m +over +no_defs"> <!-- XML -->

The "srs" attribute here stands for [Spatial reference system](http://en.wikipedia.org/wiki/Spatial_referencing_systems). In the "srs" above, note the specification of a map projection and a [datum](http://en.wikipedia.org/wiki/Datum_(geodesy) ) type.

TODO -- more text here.

For more information on map projections, read the proj.4 documentation, or check out this [guide to selecting map projections](http://www.georeference.org/doc/guide_to_selecting_map_projections.htm).

Related links

 * http://spatialreference.org/about/
 * http://en.wikipedia.org/wiki/Map_projection
 * http://trac.osgeo.org/proj/wiki/GenParms

## Map Design

For a longer treatment of MapDesign in Mapnik, see the [MapDesign page](MapDesign).

For more a detailed treatment of map design, see the written works of [Cynthia Brewer](http://www.personal.psu.edu/cab38/) and [Alan MacEachren](http://www.google.com/search?q=alan+maceachren+books).
