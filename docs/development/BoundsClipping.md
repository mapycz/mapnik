# BoundsClipping

This is a page to document bounding box clipping in the Mapnik codebase.

Clipping is used primarily to ensure `valid` and appropriate coordinate bounds are sent to layer datasource plugins when fetching features to render, and is necessary when checking for intersection between a map bbox and its layers when projections differ (since proj4 will fail if invalid extents are used for a given projection).

Clipping can interact in subtle and important ways with a variety of other map and layer level parameters, and became more error prone (but powerful) with the introduction of support for Proj4 in r330.

### Tickets related to bounding box clipping over time include:
 
* [#127](https://github.com/mapnik/mapnik/issues/127) BBox clipping fails when there's no intersection
* [#204](https://github.com/mapnik/mapnik/issues/204) map.buffer_size() does not seem to fetch data within buffer zone
* [#308](https://github.com/mapnik/mapnik/issues/308) Mapnik bug when features with invalid coordinates for target projection are encountered
* [#402](https://github.com/mapnik/mapnik/issues/402) Some points from PointDatasource get lost on reprojection
* [#486](https://github.com/mapnik/mapnik/issues/486) Bounds clipping likely to fail if using map_buffer after r1348
* [#506](https://github.com/mapnik/mapnik/issues/506) Add a Map level custom extent or max_extent to enhance clipping ability
* [#548](https://github.com/mapnik/mapnik/issues/548) Layer query bbox not correct on Lambert Map
* [#549](https://github.com/mapnik/mapnik/issues/549) Problem with local projections and worldwide data extent
* [#751](https://github.com/mapnik/mapnik/issues/751) Better handling of bounding box transform (avoids need for 'maximum-extent').

### Key Changesets include:

* r522 - initial impl of clipping in feature_style_processor.hpp
* r770 - conditional use of proj_transform and clipping
* r789 - Check for layer intersection
* r851 - ensure clipping always happens against layer extent
* r853 - switched to use the buffered map extent (map bbox + map.buffer_size)
* r1348 - changed bounds clipping to buffered map extent, created [#486](https://github.com/mapnik/mapnik/issues/486)
* r2776 - added maximum-extent attribute to map ([#506](https://github.com/mapnik/mapnik/issues/506))
* r2782 - if maximum-extent is provided, clip map query to it ([#506](https://github.com/mapnik/mapnik/issues/506)), which provides one potential, performant, solution to [#549](https://github.com/mapnik/mapnik/issues/549)
* r2784 - *switch* to first attempt to clip and intersect against map extent, then falling back to layer extent
* The fallback to layer clipping should now be rarely needed except where a map buffer pushes potential map extent outside of valid bounds but this can now be solved by supplying `maximum-extent` ([#506](https://github.com/mapnik/mapnik/issues/506))
* r2785 - skip rendering specific coordinates which cannot be back projected from the layer srs into the map srs, avoiding the previous outcome of full skipping of features (this is at the cost of potentially odd looking polygons, but should not impact performance)
* r3048 - fixed bug in fallback intersection checking that lead to false positive and uneeded clip

### IRC discussions:

Discussion of fixing clipping after addition of map_buffer to avoid truncated text at tile edges:

* introduction of `maximum-extent`: http://www.mail-archive.com/mapnik-devel@lists.berlios.de/msg00913.html
* http://mapnik.dbsgeo.com/mapnik_logs/2009/02/01/ - prediction of [#486](https://github.com/mapnik/mapnik/issues/486)