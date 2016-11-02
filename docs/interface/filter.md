# Filter

Each Style-Rule can optionally have a Filter attached. Mapnik walks through all Rules of a Style and checks if it has a Filter specified and if this Filter matches the Object currently rendered. Filters compare a Feature's attributes against the specified rules. When the Datasource is a Postgis Database, the Filter operates on the tables columns, for Shapefiles the dbf columns are used.

In XML [character entities](http://en.wikipedia.org/wiki/List_of_XML_and_HTML_character_entity_references) are used to construct filters. You can use the following characters to specify value-comparisons:

 * Greater Than: `&gt;`
 * Greater Than or Equal: `&gt;=`
 * Less Than: `&lt;`
 * Less Than or Equal: `&lt;=`
 * Equal: `=`

Filters can be combined with the following operators:

 * A `and` B
 * A `or` B
 * `not` A

And they can be combined in complex rules using brackets: `(` and `)`.

Attributes can be compared against [Regular expressions](http://en.wikipedia.org/wiki/Regular_expression) using the `.match` operator.

Following operators, functions and constants are also supported:
 * Operators: `+`, `-`, `*`, `/`
 * Unary functions: `sin`, `cos`, `tan`, `atan`, `exp`, `log`, `abs`, `length`
 * Binary functions: `min`, `max`, `pow`
 * Constants:
  * `null`
  * `false`
  * `true`
  * `point` = 1
  * `linestring` = 2
  * `polygon` = 3
  * `collection` = 4
  * `pi` = 3.1415926535897932384626433832795
  * `deg_to_rad` = `pi` / 180 = 0.017453292519943295769236907684886
  * `rad_to_deg` = 180 / `pi` = 57.295779513082320876798154814105

## Examples in XML
Matches all objects that have an attribute "amenity" with a value of "restaurant":

```xml
    <Filter>[amenity] = 'restaurant'</Filter> 
```

Match if a value is NULL:
```xml
    <Filter>[amenity] = 'restaurant' and not ([name] = null)</Filter> 
```

Also, doing modulo is possible:
```xml
    <Filter>[height] % 50 = 0</Filter> 
```

NEW in Mapnik 2.1.x: Matches all features that contain point geometries:

```xml
    <Filter>[mapnik::geometry_type]=point</Filter> 
```

Note: the geometry types that can be matched include: `point`,`linestring`,`polygon`, or `collection` (multiple different types per feature).

Matches all Objects that have an attribute "CARTO" with a value that compares greater or equal 2 and lower then 5:

```xml
    <Filter>[CARTO] &gt;= 2 and [CARTO] &lt; 5</Filter>
```

Matches all Objects that have an attribute "waterway" with a value of "canal" a) without a "tunnel" attribute or b) with a "tunnel" attribute that has a value different from "yes" and "true".

```xml
    <Filter>[waterway] = 'canal' and not ([tunnel] = 'yes' or [tunnel] = 'true')</Filter> 
```

Example using an Regular expression, matching all Objects with an attribute "place" with a value of "town" and an attribute "population" with a value consisting of exactly 5 characters where the first one is one of 5, 6, 7 or 8 and the remaining 4 characters are digits.

```xml
    <Filter>[place] = 'town' and [population].match('[5-9]\d\d\d\d')</Filter>
```

## Examples in Python
In python filters can be set using the following syntax:

```python
    f = Filter("[name] = 'value'")
```

## See also
 * The [OpenStreetMap Stylesheet](http://trac.openstreetmap.org/browser/applications/rendering/mapnik/osm.xml?rev=9228) which uses Filters in many ways.
 * The special Filters [ElseFilter](https://github.com/mapnik/mapnik/wiki/ElseFilter) and [AlsoFilter](https://github.com/mapnik/mapnik/wiki/AlsoFilter)
