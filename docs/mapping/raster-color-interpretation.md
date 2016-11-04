# Raster color interpretation

Reading raster data into a mapnik::raster requires interpreting it as either RGB(A) or data.
When the style uses a [[RasterColorizer]], a "data" interpretation is expected.
Otherwise an "RGB(A)" interpretation is expected.

The only raster plugin supporting "data" interpretation is currently the [[GDAL]] one.
The [[PgRaster]] plugin under development at time of writing is currently using the same strategy used by the GDAL plugin to decide how to interpret the input rasters. But such strategy has limitations. For example it cannot re-order RGB(A) bands, nor find a grayscale in a band which is not the first one.

This page is an attempt to standardize bands extraction and interpretation in a way that is backward compatible and improves flexibility. Plugins that are willing to adhere to this specification would accept a "band" parameter as the one supported by GDAL but with extended semantic.

The band parameter could have the following values:

 - `rgb[:<r>,<g>,<b>]`

   Red, green and blue channels found in input.
   Where `<r>`, `<g>` and `<b>` are integer representing 1-based band indexes.

 - `rgb`

   Same as `rgb:1,2,3`

 - `rgba[:<r>,<g>,<b>,<a>]`

    Red, green, blue and alpha channels found in input.
    Where `<r>`, `<g>`, `<b>` and `<a>` are integer representing 1-based band indexes.

 - `rgba`

    Same as `rgba:1,2,3,4`

 - `g[:<n>]`

   Grayscale channel found in input.
   Where `<n>` is an integer representing 1-based band indexes.

 - `ga[:<g>,<a>]`

   Grayscale and alpha channels found in input.
   Where `<g>` and `<a>` are integer representing 1-based band indexes.

 - `d[:<n>]`

   Input values should not be interpreted by the reader.
   [[RasterColorizer]] could then be used to map values to colors.
   Where `<n>` is an integer representing 1-based band indexes.

 - `<n>`

   Same as `d:<n>`.
   **This is for backward compatibility with the GDAL plugin**

 - `auto`

   The default value, if band parameter is omitted, enables guess based on input raster:
   - If the input has 1 band, `g` assumed
   - If the input has 2 band, `ga` assumed
   - If the input has 3 bands, `rgb` assumed
   - If the input has 4 bands, `rgba` assumed

 - `-1`

   Same as `auto`.
   **This is for backward compatibility with the GDAL plugin**

 - `0`

   Same as `auto`.
   This is for consistency (since band numbers are 1-based)
