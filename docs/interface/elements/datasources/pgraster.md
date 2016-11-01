# PgRaster plugin

This plugin supports reading raster datasets from [PostGIS](http://postgis.net).

# Installation

Make sure that running `python scons/scons.py` shows the following line

    Checking for pg_config... yes

To check if the gdal plugin built and was installed correctly you can do:

```python
    >>> from mapnik import DatasourceCache as c
    >>> 'pgraster' in c.plugin_names()
    True
```

## Parameters

| *parameter*       | *value*  | *description* | *default* |
|:------------------|----------|---------------|----------:|
| host                  | string       | name of the postgres host | |
| port                  | integer      | name of the postgres port | |
| dbname                | string       | name of the database | |
| user                  | string       | username to use for connecting | |
| password              | string       | user password to use for connecting | |
| table                 | string       | name of the table to fetch, this can be a sub-query;  subquery has to use syntax of:  '( ) as subquery'. | |
| raster_field          | string       | name of the raster field, in case you have more than one in a single table. | Deduced from metadata tables if possible |
| raster_table          | string       | name of the table containing the returned raster; for determining SRIDs with subselects | |
| srid                  | integer      | srid of the table, if this is > 0 then fetching data will avoid an extra database query for knowing the srid of the table | 0 |
| extent                | string       | maxextent of the rasters | determined by querying the metadata for the table |
| extent_from_subquery  | boolean      | evaluate the extent of the subquery, this might be a performance issue | false |
| estimate_extent       | boolean      | estimate extent from statistics table if not specified | false |
| connect_timeout       | integer      | timeout is seconds for the connection to take place | 4 |
| persist_connection    | boolean      | choose whether to share the same connection for subsequent queries | true |
| row_limit             | integer      | max number of rows to return when querying data, 0 means no limit | 0 |
| cursor_size           | integer      | if this is > 0 then server cursor will be used, and will prefetch this number of features | 0 |
| initial_size          | integer      | initial size of the stateless connection pool | 1 |
| max_size              | integer      | max size of the stateless connection pool | 10 |
| prescale_rasters      | boolean      | whether to automatically scale input rasters | false |
| use_overviews         | boolean      | whether to use raster overviews when available | false |
| clip_rasters          | boolean      | whether to automatically clip input rasters | false |
| max_async_connection  | integer       | max number of PostGIS queries for rendering one map in asynchronous mode. Full doc [here](Postgis-async). | 1 |
| band        | integer  | request for a specific raster band index (1-based). 0 means to read all bands. Note that a band read from a single band raster gets interpreted as Grayscale if band=0 is specified while they retain their original value when explicitly referenced with the "band" parameter. This affects effectiveness of [[RasterColorizer]]  | 0 |


# Styling

To style a layer from PgRaster use the [[RasterSymbolizer]]
