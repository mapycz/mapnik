There are several gochas related to SVG marker rendering and visual quality.

This document applies to Mapnik 2.1 - 2.2. Other versions may behave differently.

# Reasons for potentially blurry rendering

In short Mapnik is not great a pixel fitting icons (see http://dcurt.is/pixel-fitting for background).

Bilinear resampling is used for rendering raster markers. So if the markers are scaled to a size that is not cleanly divisible into the original size then resampling will not produce a crisp result.

With SVG rendering the same kind of blurriness can result if the final rendered dimensions of the svg are not cleanly divisible. This is a common problem if you attempt to set the width/height of a marker symbolizer using svg because the scaling is done against the bounding box of the svg paths (see below for more details).

# Setting width/height properties

These properties, when set on a MarkersSymbolizer, allow rendering the marker at the specified dimensions, even if it differs from the source marker dimensions. But the behavior is slightly different depending on whether an svg file is loaded, a raster marker (like png,jpeg,tiff) is used, or if an ellipse is drawn dynamically.

And internally supplying a width/height means Mapnik will attempt to set up a scaling transformation between the known width/height and the new width/new height such that `sx = <new width>/<original width>` and `sy = <new height>/<original height>`. This is meaningful because you can also apply the property of `marker-transform: scale(sx,sy)` to directly scale the marker. If both are supplied they are combined multiplicatively with the transform being applied second.

## Ellipse drawing

An ellipse marker is draw by default if no `marker-file` is specified. In this case a 10x10 px circle will be drawn by default. If the `marker-width` is specified then a circle will be draw at that width and the height will equal the width. You can achieve an ellipse with a different height and width if you supply both parameters.

## SVG file

If an svg is loaded from the file and the user supplies a width or height value then the scaling transforms will be set up against the svg bbox dimensions. NOTE: these dimensions are not the same as the `<svg width=XXX height=XXX>` values which mapnik ignores (for more details see https://github.com/mapnik/mapnik/issues/1122).

If only the width or height is supplied then only a single scaling transform is set up for the entire marker from the relative dimension. If both are supplied then both a `sx` and `sy` is applied to the marker allowing it to be scaled differently in both dimensions.

## Raster file

If a png, jpeg, or tiff image is loaded from a file and the user supplies a width or height value then behavior is the same as svg loaded except that the scaling transform will be set up against the image dimensions and alpha pixels will not be used to try to clip the extent of the image.