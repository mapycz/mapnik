# Aspect Fix Mode

Mapnik's `aspect_fix_mode` is both wonderful and evil. This page explains why.

### Background

Mapnik uses a bounding box (aka. envelope or bounding rectangle) to decide what data to query and where to place it on a given map.

A bounding box (BBOX) is made up of 2 x,y coordinate pairs representing the lower left corner and the upper right and is expressed as `minx,miny,maxx,maxy`. So a BBOX that represents the entire world is `-180,-90,180,90` if we are speaking in long/lat degrees. The coordinate values are specific to the spatial reference system you are using, so another common BBOX is the global extents in [web mercator](http://wiki.openstreetmap.org/wiki/Mercator): `-20037508.342789244,-20037508.342789244,20037508.342789244,20037508.342789244`. Curious how these were calculated? See [this code](https://github.com/mapbox/tilelive-mapnik/blob/2af055024e74414e75c714cbd47a115f43cfb3f2/lib/render.js#L8-L12).

With common tiling schemes it is easy to know the BBOX based on a given zoom level and x/y tile id and to know the reverse. Learn more at <http://www.maptiler.org/google-maps-coordinates-tile-bounds-projection/> and see some code that does this at <https://github.com/mapbox/node-sphericalmercator>.

### Are BBOX's square?

They can be, but often are not. If you look back above you'll notice that the mercator bounding box is square, which makes sense because the web mercator projection specifically excludes latitudes ±85.05113° to make web mercator work well in a [square tiling scheme](http://wiki.openstreetmap.org/wiki/Slippy_map_tilenames)

### Are rendered map images square?

They can be, but often are not. Mapnik allows the user to control the width and height of a rendered image. While tiling renderers usually only allow images that are 256x256, Mapnik allows any width or height greater than 0.

### Aspect ratio of BBOX's and the rendered map

If your BBOX is square then `(maxx-minx)/(maxy-miny)` will equal 1 meaning your [aspect ratio](http://en.wikipedia.org/wiki/Aspect_ratio_(image)) is `1:1`. If your rendered map `width` is equal to its `height` then your map is also square with an aspect ratio of `1:1`.

### What happens when the aspect ratio of the BBOX or map are not 1:1?

Mapnik automatically adjusts the BBOX. So, the aspect ratio of the map width and height is calculated and used to grow the appropriate dimension of the BBOX so that the ratios match.

This default internally in Mapnik is called `aspect_fix_mode.GROW_BBOX` and is the default because of [this line](https://github.com/mapnik/mapnik/blob/b315eb2167b08a302ef4c8b20db69d23e9cc070c/src/map.cpp#L72). The other possible behaviors are to shrink the BBOX, or grow or shrink the map dimensions. All the options can be see [here](https://github.com/mapnik/mapnik/blob/b315eb2167b08a302ef4c8b20db69d23e9cc070c/src/map.cpp#L53-L59). Whenever the map bbox is set or the image dimensions are modified [this code](https://github.com/mapnik/mapnik/blob/b315eb2167b08a302ef4c8b20db69d23e9cc070c/src/map.cpp#L72) is triggered to "fix" the aspect ratios.

### How to change the defaults

In python:

Say we wanted to change the mode to `ADJUST_BBOX_HEIGHT` and we have a map in memory with the variable `m`:

C++:
```cpp
m.set_aspect_fix_mode(mapnik::Map::ADJUST_BBOX_HEIGHT);
```

Python:
```python
m.aspect_fix_mode = mapnik.aspect_fix_mode.ADJUST_BBOX_HEIGHT
```
For a python example of this in action see <https://gist.github.com/andrewharvey/1290744>

In Javascript [this is not yet exposed](https://github.com/mapnik/node-mapnik/issues/177).
