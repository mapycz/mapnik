<!-- Name: Troubleshooting -->
<!-- Version: 6 -->
<!-- Last-Modified: 2011/07/22 11:07:27 -->
<!-- Author: aengus -->


# Troubleshooting

Make sure to check out LearningMapnik for tips on certain topics, and ExampleCode for hints on not-yet-well-documented features.


## Global data has odd lines wrapping across it

 * *Description*: You get a shapefile in WGS84 and try to display it on a map in Google Mercator projection. But, the data has coordinates that wrap the dateline (180˚ meridan) in the wrong way.

[[/images/bogus_lines.png]]

 * *Solution*: This shapefile is bogus, and you need to fix it. *Usually* this problem can be fixed by clipping the extents of the shapefile before rendering with Mapnik. An easy way to do this is using the new clipping feature in GDAL 1.7. So, assuming a shapefile in WGS 84/EPS:4326 called 'sketchy_natural_earth.shp', fix it by doing:

```sh
    $ ogr2ogr fixed.shp sketchy_natural_earth.shp -t_srs EPSG:900913 -clipsrc -180 -90 180 90
```

  * Note: if you get a long usage error it likely means you are not running GDAL 1.7!

  * Note: the '-t_srs EPSG:900913' also reprojects it to Google mercator, which will allow Mapnik to render it faster. But the real reason we do this is to watch for whether ogr2ogr throws further errors. If is does, then your shapefile really as problems, and likely contains coordinates that are outside the value extent for Google (Spherical) Mercator. If this is the case then try:

```sh
    $ ogr2ogr fixed.shp sketchy_natural_earth.shp -skipfailures -t_srs EPSG:900913 -clipsrc -180 -90 180 90
```

  * The '-skipfailures' tells ogr2ogr to skip any features containing bogus coordinates, which is good because they will potentially cause mapnik to render fewer shapes that you'd like (see [#308](https://github.com/mapnik/mapnik/issues/308)).

  * Now, if that results in a shapefile missing key chunks of data, then we need to up the ante and call in some sort of tool to actually clean the topology. You may know of some - in our experience importing into PostGIS, cleaning there, then exporting again to a shapefile works nicely. The clean script comes from Horst Duester and can be downloaded from here: http://www.sogis1.so.ch/sogis/dl/postgis/cleanGeometry.sql. Basically the dance looks like:

```sh
    # download the one with the crazy
    wget http://www.naturalearthdata.com/http//www.naturalearthdata.com/download/10m/physical/10m-land.zip
    unzip 10m-land.zip
    
    # create a temp db
    createdb -T template_postgis spring_cleaning
    
    # grab the magic topology function
    wget http://www.sogis1.so.ch/sogis/dl/postgis/cleanGeometry.sql
    
    # import function into your db
    psql spring_cleaning -f cleanGeometry.sql
    
    # load the shapefile into postgis
    shp2pgsql -s 4326 10m_land.shp land | psql spring_cleaning
    
    # fix the shapefiles geometry column, prepare for lots of noise
    psql spring_cleaning -c "update land set the_geom = cleanGeometry(the_geom);"
    
    # dump back to a shapefile
    pgsql2shp -f cleaned_but_still_sketchy.shp spring_cleaning land
    
    # then, finally, clip it and hope that you don't get too many errors from og2ogr and your shapes look okay
    ogr2ogr fixed.shp cleaned_but_still_sketchy.shp  -f "ESRI Shapefile" -skipfailures -t_srs EPSG:900913 -clipsrc -180 -90 180 90
```

A bit more advanced version of this converted to a script can be found at https://github.com/springmeyer/seaweed/blob/master/clean_natural_earth.py

If this does not work, then go back to the person that gave you this shapefile and return it. Or check out http://trac.osgeo.org/postgis/wiki/UsersWikiCleanPolygons and see if any gurus have added new scripts.

## OGCServer Fails to Load External Entities

 * *Description*: OGCServer won't start after installation and configuration when using XML styling with external entities, but nik2img can still parse the files OK. Error message in server log: "I/O warning : failed to load external entity %entities;^"

 * *Solution*: Change all the paths in the XML style files AND external entity files from relative to absolute. Ensure line endings are the correct format for your operating system.  I encountered this problem when porting my code from windows to mac.  The source of the problem was found by commenting out most of the layers and scrutinising the file paths, it turned out that some of them were incorrect.
