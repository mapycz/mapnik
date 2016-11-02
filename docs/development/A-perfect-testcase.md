Solving hard software bugs for developers can be time consuming, requiring many hours of uninterrupted focus. When time is short the most important ingredient to properly solving bugs is good a testcase.

### Anything helps, but a testcase best

If you are a user reporting a bug just reporting the bug is enough. But if you can afford the time to create a good testcase for the bug then the likelyhood of it being solved is much greater.

A good testcase enables the bug to be replicated and quickly isolated. And once a bug is solved a good testcase can be translated into a [regression test](http://en.wikipedia.org/wiki/Regression_testing) to ensure that the bug never reappears again without quickly being noticed. A good testcase makes the hardest of bugs approachable by anyone. The lack of a good testcase can make the easiest of bugs potentially out of reach to solve for even the developer most familiar with the code because that developer may not have time or energy to create a testcase to replicate the bug. Yes, creating a testcase often takes more time than solving the actual bug. And creating a good regression test can also take more time than solving the bug.

## So what is a good testcase?

Here are some attributes that capture good testcases for Mapnik:

### Small

The data involved in the testcase should ideally be small enough that it is fast to download. If the bug can be replicated with data no larger than 1 MB that is the ideal.

### Portable

The testcase should be easy to set up. This means that any data should be referenced locally in your stylesheet and should be converted to file based formats and moved out of databases. It also means a small README explaining how to set up the data to replicate is important.

If the bug does not relate to png image symbols make sure to strip them from the stylesheet.

If the bug is replicable no matter what font is use make sure to use Mapnik's default font of `DejaVu Sans Book`.

If the bug only occurs when data is being pulled from a database then try dumping the data to a file and reloading it: if you can still replicate the bug this way then post the file-based data (like a shapefile or CSV) and instructions for loading the data into a database. 

CSV testcases are ideal because the [Mapnik CSV Plugin](https://github.com/mapnik/mapnik/wiki/CSV-Plugin) supports reading data from a `inline` string in the XML. This allows the Mapnik XML to be completely standalone.

Here is an example of using a WKT string with a CSV datasource inside an XML to create a map that could serve as a testcase for point rendering: https://gist.github.com/springmeyer/7459452. A simplified version is:

```xml
   <Layer name="layer" srs="+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs">
        <StyleName>point</StyleName>
        <Datasource>
            <Parameter name="type">csv</Parameter>
            <Parameter name="inline">
id|name|wkt
1|null island|Point(0 0)
            </Parameter>
        </Datasource>
    </Layer>
```

### Simple

If it takes a datasource with 1000 rows, 40 attributes, and mixed geometry types to replicate the bug then that's okay. However if you can reduce your data to one row, one attribute, and a single geometry type and still hit the bug then that is a vastly simpler testcase. As a user you already have 1) the data locally, 2) an understanding of the data and what is consists of and 3) and have already "replicated" the bug. So as a user you are in the ideal position to reduce and simplify a testcase before posting your bug report. If you don't then someone else will need to reduce the testcase before trying to solve the issue and its likely it will take them much longer than it would take you to reduce it.

Here is an practical example: Say you have identified a labeling bug when rendering multipolygons from PostGIS like [this report](https://github.com/mapnik/mapnik/issues/2062). The testcase attached to that bug is excellent because it provides the data in nice zipped package and Mapnik stylesheets ready to run with it. But it could have been much simpler and smaller. The testcase includes `4492` rows when testing shows that one row would have been enough to replicate the original labeling problem. I reduced the testcase to just one row and converted to CSV format data, confirmed the bug was still present, and now we have a simple and portable testcase as a reference for solving the bug and created a regression test. I exported data from postgres to a CSV which Mapnik could read like:

```sh
# create CSV headers
echo 'wkt|label' > out.wkt
# dump a single multipolygon as WKT and create a `label` column for labeling
psql rumap_tc -c "select ST_AsText(geom) as wkt, 'label' as label from topo_sz where id=697611;" -A -t >> out.wkt
```

The XML testcase then was:
```xml
<Map srs="+proj=merc +datum=WGS84 +over">
<Style name="poly">
  <Rule>
    <PolygonSymbolizer/>
    <TextSymbolizer face-name="DejaVu Sans Book">label</TextSymbolizer>
  </Rule>
</Style>
<Layer name="poly" srs="+proj=merc +datum=WGS84 +over">
    <StyleName>poly</StyleName>
    <Datasource>
       <Parameter name="type">csv</Parameter>
       <Parameter name="file">out.wkt</Parameter>
    </Datasource>
  </Layer>
</Map>
```

The resulting `out.wkt` was a large so I held back from embedding inline in the CSV, but for testcases replicable with very small data inlining is ideal. Here is a full example that uses inline WKT data in CSV: https://gist.github.com/springmeyer/7459452

### Describes process

How to replicate the bug matters: how did you end up hitting it? What where the circumstances? Did it occur right when you started using Mapnik or after many hours? What operating system are you on? What Mapnik version? Does it impact all Mapnik versions you've tried? Did you just re-install Mapnik before hitting the bug?
