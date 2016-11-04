# Managing complex map files using XML entities

**Note**: you will need a copy of mapnik built against libxml2.  This is not the default, please read the **Mapnik XML Support** or [issue here](https://github.com/mapnik/mapnik/issues/3191) for details.

Mapnik XML files can become quite complex. This tutorial introduces some
techniques to keep large map files more maintainable. Specifically it
demonstrates how to avoid duplicate data in the XML file, like:

* color values
* database connection parameters
* icon directories

It also shows how to split a single, monolithic map file into reusable
components using external entities and XInclude.

## Mapnik XML support

Mapnik currently supports three different XML parsers:

* the boost spirit based parser
* the tinyxml parser
* libxml2

The three parsers differ in size, external dependencies and the number of XML
features they support. The most comprehensive parser is the libxml2 parser and
it is the only one that supports XML entities.  As of Mapnik 0.6.0 libxml2 is the default
when building the Mapnik source with SCons, and available in the Windows binaries as of 0.6.1.

## Compiling mapnik with libxml2 support

If not default in your Mapnik version (< 0.6.0) the libxml2 parser is enabled by setting the XMLPARSER option at
compile time:

```sh
    $ python scons/scons.py configure XMLPARSER=libxml2
```

Of course this requires the libxml2 library and, depending on the distribution
the corresponding devel package. If `xml2-config` is not in the PATH its location
can be set using the `XML2_CONFIG` option.

For example, if you have installed the latest libxml2 on mac os x via Macports, you might need to do:

```sh
    $ python scons/scons.py configure XML2_CONFIG=/opt/local/bin/xml2-config
```

## Internal Entities

All XML parsers have some built-in entities to escape otherwise illegal
characters:

* `&gt;`
* `&lt;`
* `&amp;`
* `&quot;`
* `&apos;`

The XML document type definition (DTD) provides a way to declare new, user
defined entities:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <!DOCTYPE Map[
        <!ENTITY water_color "#b5d0d0">
    ]>
    <Map bgcolor="&water_color;"/>
```

This XML document declares an internal entity named water_color. This entity is
referenced by the bgcolor attribute of the Map element. The parser replaces all
occurrences of `&water_color;` with the string `#b5d0d0`.

Using entities for common values results in a single point of definition. This
greatly improves maintainability. Instead of searching and replacing all
occurrences of a value, there is a single place to change it. By using names,
like water_color the XML becomes more readable which helps a lot with future
changes. In case of color values it has an additional benefit. Because all
entities are declared at the top of the document the color set of the map is
immediately apparent. Of course color values are not the only application. Any
reoccurring value is a candidate for an entity.

It is allowed to nest entities:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <!DOCTYPE Map[
        <!ENTITY home_dir "/home/david">
        <!ENTITY icons    "&home_dir;/map/icons">
    ]>
    <Map>
        <Style name="volcanos">
            <Rule>
                <PointSymbolizer file="&icons;/volcano.png"
                                 type="png" width="16" height="16"/>
            </Rule>
        </Style>
    </Map>
```

However, these internal entities are not suitable for larger blocks. They also
do not help with sharing common styles and layers between different maps.

## External Entities

External entities are declared by adding the keyword SYSTEM:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <!DOCTYPE Map[
        <!ENTITY db_settings SYSTEM "settings/db_settings">
    ]>
    <Map>
        <Layer name="volcanos" status="on">
            <StyleName>volcanos</StyleName>
            <Datasource>

                <Parameter name="table">volcanos</Parameter>

                &db_settings;

            </Datasource>
        </Layer>
    </Map>
```

The entity declaration assigns the content of the file `settings/db_settings` to
the entity `&db_settings;`. When parsed the reference to `&db_settings;` in the
Datasource section is expanded to the content of the file. If a relative
filename is given the file is searched relative to the document. The file
`settings/db_settings` could look like this:


```xml
    <Parameter name="type">postgis</Parameter>
    <Parameter name="host">www.example.org</Parameter>
    <Parameter name="port">5433</Parameter>
    <Parameter name="user">david</Parameter>
    <Parameter name="dbname">geo</Parameter>
```

Note that this is not a legal XML document on its own because it does not have
a single root element. It is a list of elements. But the tags have to be well
balanced. Also note that references to external entities are illegal in
attribute values. They are only allowed in text sections.

It is possible to use entity references in external entities. This allows a
limited form of parameterization. Consider the following example:

File earthquake.map:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <!DOCTYPE Map[
        <!ENTITY since_year "1970">
        <!ENTITY earthquakes_since SYSTEM "earthquakes_since.lay">

        <!ENTITY earthquakes_default_style SYSTEM "earthquakes.sty">
        <!ENTITY common_styles SYSTEM "common.sty">
        <!ENTITY common_layers SYSTEM "common.lay">
        <!ENTITY db_settings SYSTEM "db_settings">

        <!-- colors -->
    ]>
    <Map>
        &earthquakes_default_style;

        &earthquakes_since;

        &common_styles;
        &common_layers;
    </Map>
```

File earthquakes_since.lay:

```xml
    <Layer name="earthquakes_since" status="on">
        <StyleName>earthquakes</StyleName>
        <Datasource>

            <Parameter name="table">
                (SELECT * FROM earthquakes
                 WHERE year &gt;= &since_year; ) AS earthquakes_since
            </Parameter>

            &db_settings;

        </Datasource>
    </Layer>
```

This is a quite flexible setup. It is very easy to add and remove thematic
overlays. Other overlays may use the same parameters by referencing the
same entities. Styles can be changed by replacing the reference to
`&earthquakes_default_style;` with a custom one. It is also possible to have
many map files all referencing the same set of styles and layer files but with
different settings.

## Entities Summary

Entities provide a way to use symbolic names in the map file. This improves
readability and helps to build logical groups. By providing a single point
of definition map files are better adaptable to different environments and in
general more maintainable. External entities can store whole blocks of XML.
This helps to build reusable collections of layers and styles. These reusable
components can be parameterized using other entities as needed.

## Including external files using XInclude

libxml2 also provides support for decomposing large mapnik XML files through the use of XInclude.

To enable XInclude in your root file, modify the Map container tag, adding the xi namespace.  Adding xi:include tags where the href attribute names the file to include completes the change to the root file.

File tests/data/good_maps/xinclude/map.xml:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <!DOCTYPE Map  >
    <Map xmlns:xi="http://www.w3.org/2001/XInclude" srs="+init=epsg:4326" bgcolor="rgb(255,255,255)" >

    <!-- http://www.oreillynet.com/xml/blog/2004/05/transclude_with_xinclude_and_x.html -->

    <xi:include href="styles.xml" />

    <xi:include href="layers.xml" />

    </Map>
```

Included files wrap their content within an Include tag.  XInclude replaces the xi:include tags with the contents of the included file, yielding a single, merged XML document.
mapnik's XML parser then merges the contents of the Include tag into the Map tag, resulting in an XML document tree identical to one produced by processing a single file containing
all Style and Layer tags.

File styles.xml:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <Include xmlns:xi="http://www.w3.org/2001/XInclude">

        <Style name="point_style">
            <Rule>
                <PointSymbolizer file="../../svg/point.svg"/>
                <TextSymbolizer face_name="DejaVu Sans Book" size="12" name="[name]" halo_fill="rgb(255,255,255,100)" halo_radius="1" dy="-5"/>
            </Rule>
        </Style>

        <Style name="world_borders_style">
            <Rule>
                <PolygonSymbolizer fill="grey" gamma="0.7"/>
            </Rule>
        </Style>

    </Include>
```

File layers.xml:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <Include  xmlns:xi="http://www.w3.org/2001/XInclude">

        <Layer name="world_borders" srs="+proj=merc +lon_0=0 +lat_ts=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs">
            <StyleName>world_borders_style</StyleName>
            <Datasource>
                <Parameter name="file">../../shp/world_merc.shp</Parameter>
                <Parameter name="type">shape</Parameter>
            </Datasource>
        </Layer>

    </Include>
```

## Combining XInclude and External Entities

Used together, XInclude and External Entities provide improved document size management and parameterization, greatly reducing maintenance effort.

A common pattern found in mapnik XML file sets is specifying the zoom levels for which a Style or Layer is enabled.  Creating an external entity file assigning symbolic names to the minimum and maximum scale denominators for each zoom level increases readability.  A fragment of such a file is shown below:

file: zoomsymbols.txt

```xml
    <?xml version="1.0" encoding="utf-8"?>

    <!ENTITY zoom00max		"750000000" >
    <!ENTITY zoom00min		"540000000" >

    <!ENTITY zoom01max		"500000000" >
    <!ENTITY zoom01min		"270000000" >

    <!ENTITY zoom02max		"250000000" >
    <!ENTITY zoom02min		"130000000" >
```

Adding a DOCTYPE line to the layers.xml file to include the external entities file, and adding parameterized minzoom and maxzoom attributes to the layer yields the file below:


File layers_with_entities.xml:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <!DOCTYPE Include SYSTEM "zoomsymbols.txt">

    <Include  xmlns:xi="http://www.w3.org/2001/XInclude">

        <Layer name="world_borders" minzoom="&zoom02min;" maxzoom="&zoom00max;" srs="+proj=merc +lon_0=0 +lat_ts=0 +x_0=0 +y_0=0 +ellps=WGS84 +datum=WGS84 +units=m +no_defs">
            <StyleName>world_borders_style</StyleName>
            <Datasource>
                <Parameter name="file">../../shp/world_merc.shp</Parameter>
                <Parameter name="type">shape</Parameter>
            </Datasource>
        </Layer>

    </Include>
```

## Further Reading

* [XML.com entity tutorial](http://www.xml.com/pub/a/98/08/xmlqna0.html)
* [W3C XML Standard](http://www.w3.org/TR/REC-xml/)
* [W3C XInclude Standard](http://www.w3.org/TR/xinclude/)
* [Wikipedia XInclude Page](http://en.wikipedia.org/wiki/XInclude)
