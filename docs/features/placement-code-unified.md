# Unification of placement code

All these symbolizers have common purpose - placing labels on some geometric positions:

* `TextSymbolizer`
* `ShieldSymbolizer`
* `GroupSymbolizer`
* `MarkersSymbolizer`
* `PointSymbolizer`
* `CollisionSymbolizer`

It makes sense to allow all possible placement options for all these symbolizers. Therefore all these symbolizers supports all these placement options:

* `point`
* `interior`
* `centroid`
* `line`
* `vertex`
* `vertex-first`
* `vertex-last`
* `grid`
* `alternating-grid`

This unification is a result of great code sharing and simple [declarative description](https://github.com/mapycz/mapnik/blob/63ed040a3c1817db8b5a3f1333dccc9fb5d42f8b/include/mapnik/text/symbolizer_helpers.hpp#L57-L106) of each symbolizer.
