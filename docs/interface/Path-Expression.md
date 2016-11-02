It is an expression type that has been refined and simplified to use for file system paths.

It allows normal, static paths like `/path/to/file.png` but also dynamically constructed paths like: `/path/to/[field].png` so that for every symbolizer rendered the image/svg file path lookup would change depending on the value of a given data field, in this case an attribute called `field`.

For more info see: http://mapnik.org/news/2009/12/08/future_mapnik2/