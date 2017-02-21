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

