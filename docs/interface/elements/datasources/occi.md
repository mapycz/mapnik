# OCCI plugin

Mapnik's PluginArchitecture supports the use of different input formats.

One such plugin supports the Oracle Spatial ([Oracle Spatial](http://en.wikipedia.org/wiki/Oracle_Spatial)) extension to the popular ORACLE database.

*At the current time, only oracle version 10.2.0.x (10g) and 11.2.0.x (11g) are supported*


## Installation (On Linux)

Install the [Oracle Instant Client](http://www.oracle.com/technology/software/tech/oci/instantclient/index.html) package on your system.

Make sure you define where the includes and library files resides in _config.py_:


    OCCI_INCLUDES='/usr/lib/oracle/11.2.0.2/client/include'
    OCCI_LIBS='/usr/lib/oracle/11.2.0.2/client/lib'

Make sure that running _python scons/scons.py DEBUG=y_ shows the following line

    Checking for C++ library ociei... yes

To check if the occi plugin built and was installed correctly, try the usual Python _from mapnik import *_ on a DEBUG=y build, and look for the following debug line

    registered datasource : occi

## Troubleshooting

### double free or corruption error (oracle 10g only)

If you have problems when running the oracle input plugin and get this when accessing geometries:

    *** glibc detected *** double free or corruption (!prev): 0x08c52568 ***

go to http://www.oracle.com/technology/tech/oci/occi/occidownloads.html and download the package 10.2.0.3.0 and overwrite the libraries in your oracle instantclient install path.

Look [here](http://bugs.gentoo.org/show_bug.cgi?id=257431) for references on the problem.

### libtool error: libstdc++.so.5

    libtool error: libstdc++.so.5: cannot open shared object file: No such file or directory
Make sure you have compat-libstdc++-33 installed.


## Parameters

| *parameter*       | *value*  | *description* | *default* |
|:------------------|----------|---------------|----------:|
| host                  | string       | name of the oracle host | |
| user                  | string       | username to use for connecting | |
| password              | string       | user password to use for connecting | |
| table                 | string       | name of the table to fetch, this can be a sub-query | |
| geometry_field        | string       | name of the geometry field, in case you have more than one in a single table | GEOLOC |
| extent                | string       | maxextent of the geometries | determined by querying the oracle metadata for the table |
| row_limit             | integer      | max number of rows to return when querying data, 0 means no limit | 0 |
| row_prefetch          | integer      | number of rows to prefetch from the query before converting them to mapnik features (this allows to finetune the balance between transfer time and conversion time) | 1000 |
| initial_size          | integer      | initial size of the stateless connection pool | 1 |
| max_size              | integer      | max size of the stateless connection pool | 10 |
| use_spatial_index     | boolean      | choose wheter to use the oracle spatial index when fetching data | true |
| multiple_geometries   | boolean      | wheter to use multiple different objects or a single one when dealing with multi-objects (this is mainly related to how the label are used in the map, one label for a multi-polygon or one label for each polygon of a multi-polygon)| false |
| encoding              | string       | internal file encoding | utf-8 |

## Usage

### Python

Instantiate a datasource like:

```python
    lyr = Layer('Geometry from Oracle Spatial')
    lyr.datasource = OCCI(host='localhost',user='scott',password='tiger',table='worldborders',geometry_field='geom')
```

### XML

If you are using XML mapfiles to style your data, then using a Oracle datasource looks like:


```xml
      <Layer name="countries" status="on" srs="+proj=latlong +datum=WGS84">
        <StyleName>countries_style_label</StyleName>
        <Datasource>
          <Parameter name="type">occi</Parameter>
          <Parameter name="host">localhost</Parameter>
          <Parameter name="user">scott</Parameter>      
          <Parameter name="password">tiger</Parameter>
          <Parameter name="table">worldborders</Parameter>
          <Parameter name="geometry_field">geom</Parameter>
          <Parameter name="extent">-180,-90,180,89.99</Parameter>
        </Datasource>
      </Layer>
```

### C++

Plugin datasource initialization example code can be found on PluginArchitecture.

A OCCI datasource may be created as follows:

```cpp
    {
        parameters p;
        p["type"]="occi";
        p["host"]=oracle_server_hostname;
        p["user"]=oracle_server_username;
        p["password"]=oracle_server_password;
        p["table"]="worldborders";
        p["geometry_field"]="geom";
        p["extent"]="2309554.99818767,5024797.73763923,2318414.90507308,5040447.94690007"; // optional
    
        Layer lyr("World Borders");
        lyr.set_datasource(datasource_cache::instance()->create(p));
        lyr.add_style("worldborders");
        m.addLayer(lyr);
    }
```
