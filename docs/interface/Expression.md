Mapnik Expressions enable data-driven behavior in Mapnik. They are constructed from strings, can hold any kind of type of data, and are evaluated at render time. They are used by symbolizers like the [[TextSymbolizer]] to dynamically construct text labels for display and they are also the basis for how Mapnik [Filters](Filter) work.

Recently, in Mapnik 2.1, expressions became enabled in the `transform` parser used to apply affine transformations to image and svg symbols. So, now you can not only, for example, rotate symbols with a transform like `transform="rotate(45)"` but the rotation can be data driven like `transform="rotate([field])"`

For more info on the original design see: http://mapnik.org/news/2009/12/08/future_mapnik2/

in the feature,I think the color attribute such as the stroke of 
LineSymbolizer, the fill of PolygonSymbolizer, the fill of TextSymbolizer and so on, should also support Expression, this is more useful.

