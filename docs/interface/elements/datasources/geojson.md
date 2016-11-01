# GeoJSON plugin

This plugin can read GeoJSON. Previously GeoJSON reading in Mapnik was only supported through the [[OGR]] Plugin. But Mapnik 2.1 now internally has support for in-memory RTREE indexes and a robust JSON parser - so why not provide an extremely fast native GeoJSON plugin?

For more details on the motivations and design of this plugin see: https://github.com/mapnik/mapnik/issues/1413


