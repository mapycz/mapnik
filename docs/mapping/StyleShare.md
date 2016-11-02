<!-- Name: StyleShare -->
<!-- Version: 15 -->
<!-- Last-Modified: 2011/05/16 07:42:34 -->
<!-- Author: Harry Wood -->

# Sharing Map Styles

We need a proper app to share styles (Dane has started django code but its not ready yet)

The idea behind this app is that all styles uploaded will have a sample rendered live, therefore:
  * All styles must have relative paths or urls to resources
  * Must either work with OSM data or work against uploaded sample WKT or GeoJSON
  * Will be able to be downloaded as MML if that is their native format or exported as XML
  * Will also be able to be serialized as python pickles (in the future)

For now, list your styles here!

## Name of Style(s)

*Description*: A unique descriptive for your style or set of styles.

*Type*: Cascadenik MML, Mapnik XML, or Python code 

*URI*: If you host them somewhere (like github) then paste the url
  * otherwise, paste inline...

*Minimum Mapnik Version*: What Mapnik version do these style depend on?



## OpenStreetMap "standard" style

*Description*: Current stylesheet used by openstreetmap.org for the "standard" default map

*Type*: CartoCSS

*URI*: https://github.com/gravitystorm/openstreetmap-carto
http://trac.openstreetmap.org/browser/applications/rendering/mapnik/osm.xml



## OpenStreetMap France

*Description*: http://tile.openstreetmap.fr  An early fork of the "standard" cartoCSS OpenStreetMap style with various interesting refinements including more shops, and fat roads and other details at zoom 19.

*Type*: CartoCSS

*URI*: https://github.com/cquest/osmfr-cartocss


## Humanitarian style

*Description*: A style by the Humanitarian OpenStreetMap Team emphasising details for developing world/crisis features from the "Humanitarian Data Model". [More on the HOT blog]( http://hot.openstreetmap.org/updates/2013-09-29_a_new_window_on_openstreetmap_data)

*Type*: CartoCSS

*URI*: https://github.com/hotosm/HDM-CartoCSS


## OpenStreetMap Old stylesheets

*Description*: The original mapnik XML style sheet used by openstreetmap.org

*Type*: Mapnik XML

*URI*: https://github.com/openstreetmap/mapnik-stylesheets

*Minimum Mapnik Version*: Mapnik 0.7.1 (recommended)



## OSM NL Styles


*Description*: Various styles powering http://www.openstreetmap.nl/

*Type*: Mapnik XML

*URI*: http://git.openstreet.nl/index.cgi/stylesheets.git/

*Minimum Mapnik Version*: Mapnik 0.7.1 [see q](http://help.openstreetmap.org/questions/1746/running-generate_image-gives-features-only-present-in-mapnik-version-071-error)

## Mapquest

*Description*: OSM style featured on open.mapquest.com .   MIT licensed

*URI*: https://github.com/MapQuest/MapQuest-Mapnik-Style

*Minimum Mapnik Version* : 0.8.0  (see readme for other dependencies)

## Haiti OSM WMS overlay

*Description*: TODO

*Type*: Mapnik XML

*URI*: HaitiStyles

*Minimum Mapnik Version*: Mapnik 0.7.0

## Hike & Bike

*Description*: A style for hiking and biking, with hiking symbols

*Type*: Cascadenik MML

*URI*: http://mapnik-utils.googlecode.com/svn/sandbox/cascadenik/hike_n_bike/ (style.mml)

*Minimum Mapnik Version*: 0.7.0 

## By Night

*Description*: An overlay for showing lit (or non-lit) ways, buildings and areas

*Type*: Cascadenik MML

*URI*: http://mapnik-utils.googlecode.com/svn/sandbox/cascadenik/hike_n_bike/ (lighting.mml)

*Minimum Mapnik Version*: 0.6.0

## MapBox OSM Bright

*Description*: see https://github.com/mapbox/osm-bright/blob/master/README.md

*Type*: Carto MML

*URI*: https://github.com/mapbox/osm-bright.tm2 is TileMill2 reboot of https://github.com/mapbox/osm-bright/ 

*Minimum Mapnik Version*: 2.0.0

## Cascadenik Dev OSM Styles, aka "Remapniking OSM"

*Description*: demo at: http://teczno.com/cascadenik-openstreetmap-II/

*Type*: Cascadenik MML

*URI*: http://mapnik-utils.googlecode.com/svn/trunk/serverside/cascadenik/openstreetmap/

*Minimum Mapnik Version*: 0.6.1

## Tango

*Description*: tango base style file, demo at: http://osm.tcweb.org/

*Type*: Mapnik XML

*URI*: http://github.com/tclavier/mapnik

*Minimum Mapnik Version*: Mapnik 0.7.1

## WikiMedia Toolserver

*Description*: variety of styles used at http://toolserver.org/~osm/styles/

*Type*: Mapnik XML

*URI*: http://svn.toolserver.org/svnroot/p_osm/styles/

*Minimum Mapnik Version*: Mapnik 0.7.1

## Golf overlay

*Description*: A render of golf course features, for golfers.

*URI*: https://github.com/rweait/Mapnik-golf-overlay

*Minimum Mapnik Version*: Mapnik 2.0.0

## Open Streets Style

*Description*: A TileMill/Carto style for OpenStreetMap PostGIS databases that you can export later to XML Mapnik format.

*Type*: Carto MML 

*URI*: https://github.com/mapbox/open-streets-style

*Minimum Mapnik Version*:  2.2.0

## OpenStreetMap Argentina

*Description*: A localized TileMill/Carto style for OpenStreetMap PostGIS databases that you can export later to XML Mapnik format. Forked from https://github.com/gravitystorm/openstreetmap-carto

*Type*: Carto MML 

*URI*: https://github.com/osm-ar/osm-ar-carto


## VeloRoad

*Description*: VeloRoad style by Zverik is specifically made for printing in black and white at at least 150 dpi with a route overlayed. Contrast is high, labels are small and aplenty. You can see a demo at http://osmz.ru/veloroad.html

*Type*: Carto MML 

*URI*: https://github.com/Zverik/veloroad



## Hydda

*Description*: Based on an old version of OSMBright, but patched to add more features such as piers, stadiums, pitches, and more. Colors set to allow for looking good when adding transparent red-yellow-green overlays. Apache licensed. See the stylesheet in action hosted by OpenStreetMap Sweden, and [viewable via umap](http://umap.openstreetmap.fr/en/map/new/#15/51.5281/-0.1213)

*Type*: Carto MML 

*URI*: https://github.com/karlwettin/tilemill-style-hydda


## OSM swiss

*Description*: Based on OSMBright. CC-BY-SA4.0. See it here http://umap.osm.ch/de/map/vegan-in-bern_76

*Type*: Carto MML 

*URI*: https://github.com/xyztobixyz/OSM-Swiss-Style 





