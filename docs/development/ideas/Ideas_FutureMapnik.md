<!-- Name: Ideas/FutureMapnik -->
<!-- Version: 6 -->
<!-- Last-Modified: 2011/04/28 09:45:52 -->
<!-- Author: manelclos -->
# Future Features of Mapnik

## WhereCamp Session: April 4th, 2010 (Google Campus)


Attending:

 * Nino Walker & Tim Caro-Bruce (Urban Mapping | http://urbanmapping.com/)
 * Peter Keum (King County GIS)
 * Andrew Turner (GeoCommons | http://highearthorbit.com/)
 * Jeff Johnson (http://twitter.com/ortelius)
 * Dane Springmeyer (http://twitter.com/springmeyer | http://dbsgeo.com)
 * Rich Gibson (MappingHacks/Gigapan)
 * Dan Lyke (flutterby.com)
 * Brett Camper (Kickstarter | http://kickstarter.com) (formerly http://patch.com/)
 * Sylvain Carlyle (http://twitter.com/afrognthevalley)
 * Dan Dye (WeoGeo | http://weogeo.com)
 * Barbara Hui (UCLA | http://twitter.com/barbarahui)
 * Marc Pfister (http://www.drwelby.net)
 * Embeddedlinuxguy
 * Sobelito
 * Svogel186
 * add yourself if I missed you!

We met for about an hour to discuss some dream features we'd like to see in Mapnik in the year(s) to come, who's using Mapnik currently and for what, and some specific third-party tools that exist or should exist in the future for working with Mapnik.

Dane Springmeyer took notes. Please add to them if he missed anything!

## A couple key users of Mapnik
-------------------------------

GeoCommons:

 * Using Mapnik for PNG output
 * Pushing Mapnik output into GeoPDF
 * Currently running on SVG symbol support patch written by Koordinates (Robert Coup/Craig Destigter)
   * This is an issue that needs resolution (https://github.com/mapnik/mapnik/issues/320)
   * Need to consult with Artem about possibility of using AGG SVG parser in addition to librsvg+cairo

 * Actively needing C++ based WMS server
 * Have written python and ruby apps around Mapnik (see 'Lithograph' below)

Urban Mapping:

 * Visualization, query, and thematic maps api around Mapnik using Django (http://MapFluence.com)
 * Have done thinking around EC2 and OSM Elastic Block for Mapnik/OSM deployments
 * Confirmed interest in further coordinating with DevSeed and Stamen on amazon deployment wrappers
 * Have written a django dashboard to tile rendering / cache wiping / style preview as a r&d project


## Core Features of Future

Compositing/grouping of layers
------------------------------

 * WMS services need ability to flexibly group and combine layers both in rendered map and GetCaps
 * Stamen has ideas around compositing/blend modes in mapnik.Image

  - https://github.com/mapnik/mapnik/wiki/Ideas_Compositing
  - https://github.com/mapnik/mapnik/wiki/GSOC2010_Ideas
  - essentially bring TopOSM like functionality into core, expose easily in stylesheets
  - Or expose just essentials to be able to render compositions with extra python wrapper

 * UrbanMapping may have some code that does some of this already (outside Mapnik) and can possibly share


Geometry intelligent Symbolization
----------------------------------
 * More control needed to restrict certain application of symbolizer to features with given geometry type

  - e.g. add setting to be able to prevent PointSymbolizer from being applied to polygon features

 * GeoCommons usage has run into issue. Easy to work around, but would be nice to make easier to control


Better Label Placement
----------------------
 * Must have ability to have point labels shifted (by certain tolerance) for denser or better looking placement
 * Must add ability to try various placement directives, e.g. in cardinal directions (Jeff Johnson)

  - Patch for this from kosmosnimki - needs someone to step up to clean up/refactor (or potentially fund further work)
  - https://github.com/mapnik/mapnik/issues/463


Ruby Bindings
-------------
 * GeoCommons would love to have Ruby Bindings
 * Dane: We need to know from Ruby experts the best way to wrap C++ libs these days.
 * e.g. Since boost::ruby does not exist (although it has been discussed), what method to bind C++ is best?
 * Aubrey Holland has some github code that lightly wraps, but he mostly uses Python+TileCache still at this point

   - http://github.com/aub/ruby-mapnik


Not Just XML
------------

 * Support for Cascadenik (CSS) natively in Mapnik

  - one step could be moving Cascadenik python implementation into Mapnik trunk/releases
  - next step (long term) could be moving to C++ based implementation

 * Mapnik currently uses boost property_tree to handle XML (along with Parser)
 * JSON support (through boost ptree + parser) could be viable additional format - anyone interested?


Thematic Mapping
----------------
 * Expressions are needed for dynamic, data driving symbolizers

  - Excellent foundation is recently in trunk 
  - See: http://mapnik.org/news/2009/dec/08/future_mapnik2/
  - Needs more feedback, testing, funding, and additional properties exposed as expressions
 
* Urban Mapping doing quite a bit of thematic cartography in new "MapFluence" API - need more feedback about pain points


Performance
-----------
 * Mapnik is fast, but we must get faster to support web scaling
 * Profiling and benchmarks are CRITICAL
 * Opportunities upcoming:

  - Geofabrik stylesheet performance/ best practices (will be presented at SOTM)
  - Springmeyer entering Mapnik in FOSS4G WMS shootout - goal to identify bottlenecks

 * Jeff Johnson (opensgi.com)can offer data scenarios for profiling massive tile deployment situations


Next Generation Datasources
---------------------------
 * Supporting non-relational datastores

  - Cassandra, mongodb, hbase?

 * Fuller MemoryDatasource support for Lines and Polygons (currently Mapnik only supports in memory points)

  - WKT, WKB, GeoJSON reader exposed in Python

 * Implement pluggable python-based datasources (using boost::python)
 * Nino: Look into creating in-memory SQLite db using pysqlite then passing handle (not file) to Mapnik within python


##Tools

Fast C++ WMS server: **done with [[paleoserver]]**
-------------------
 * To support highly scalable, fast access to GeoCommons data as WMS
 * Many more users/orgs need this! Will broaden Mapnik community
 * Ideally could be used/prototyped by September 2010 FOSS4G "WMS shootout"
 * Multi-threaded/process server is key
 * Should build on best practices and lessons from mod_tile (separate request handling + rendering daemon)
 * Support GetMap, GetCapabilities, GetFeatureinfo, GetLegendGraphic
 * Python reference server available as a comparison - https://github.com/mapnik/OGCServer

  - Problem with Python server is that it is unmaintained currently and while Dane is developing it a bit he does not have much time to dedicate. Untested assumption that speed it critical so python will be to slow...


Single File Map (SFM)
---------------------
 * A self contained Map package - term coined by Andrew/Jeff

  - Dane has referred to this idea as "render bundle" in the past.
  - DevSeed has been brainstorming on same issues of how to package.

 * Could allow shippable GeoCommons map, e.g. download and open/browse in QGIS offline.
 * To address need of fully portable, self contained data + styles + extra display logic
 * Prototype will target Mapnik XML/Cascadenik MML + sqlite datasources + zip archive

  - Supporting varied datasources is the hard part so starting with SQLite would be best (look into pushing rasters in Rasterlite)

 * Need to start thinking through tools that can read, unpack, and pass this bundled format to render with Mapnik

  - Could be many various apps that just "understand" this packaged format for direct reading:
  - e.g. Quantumnik, WMS servers, mod_tiles, etc...


Lithograph
----------
 * Python wrapper around Mapnik that GeoCommons will share
 * Has features that push forward ideas around SFM

  - particularly the issue of moving data into SQLite
  - and the writing of MML styles from other formats


MapShift Library: work in [Millstone](http://github.com/mapbox/millstone)
----------------
 * Python library Dane Springmeyer is scoping to handle bundling existing datasources, symbols, other files that make up map
 * Perhaps could merge with Lithograph if sources were open.
 * Planning to write in python and depend on OGR/GDAL python bindings (for projection/reprojection support)
 * Will have (in Phase II) support for validation and mapfile evaluation for performance.


Quantumnik Future
-----------------
 * Currently can export out Mapnik XML and set all paths to be relative to deployed location
 * Lots more possible just within this tool to make stylesheets more portable.
 * Will be first stop to test out ideas around SFM and MapShift implementation.
 * Development lately has been driven mostly by trying to map subtle QGIS styling to Mapnik styling and adding support for new datasources
 * Future development will focus on original goals: ability to quickly author Mapnik stylesheets for one-click tile seeding and map packaging