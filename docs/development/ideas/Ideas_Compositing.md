<!-- Name: Ideas/Compositing -->
<!-- Version: 5 -->
<!-- Last-Modified: 2010/09/26 04:44:22 -->
<!-- Author: ivansanchez -->
# Compositing

[TopOSM](http://wiki.openstreetmap.org/wiki/TopOSM/Details) is pretty inspirational. How about an XML dialect for specifying the composition of multiple Mapnik-rendered layers into a single map. This example is based on [TopOSM's compositing steps](http://wiki.openstreetmap.org/wiki/TopOSM/Details#Combining_images_into_a_final_composite), which also incorporate `gdalwarp`-generated hill shadings. (These could be integrated using a [RasterSymbolizer](http://github.com/mapnik/mapnik/wiki/RasterSymbolizer), or a thin wrapper could be generated at runtime.)

* Note: see also [artem's test images](Compositing) (generated with AGG)
* Note: see also the GSOC page on related ideas part [Layer Composites](GSOC2010_Ideas)
* Note: Research possibility of use RPN (reverse polish notation) for specifying the rendering and compositing order. It would be hard to understand complex compositing schemes, but easy to implement
* Note: Research multithreaded/multiprocess rendering and compositing of layers. In a nutshell, rendering of two different layers to be composed could be handled by different threads, all compositing should be multithreaded. Research and benchmark if it makes sense performance-wise.

```xml
        <?xml version="1.0"?>
        <Composite srs="...">
    
            <!-- first, render the areas -->
            <Map href="areas.xml"/>
    
            <!-- next, composite hillshading twice: -->
    
            <!-- first, darkened and using the screen blend mode -->
            <Map type="raster" href="hillshade.tiff" blend="screen">
                <Transform type="level">70,90%,0.8</Transform>
            </Map>

            <!-- then, lightened and using the multiply blend mode -->
            <Map type="raster" href="hillshade.tiff" blend="multiply">
            <Transform type="level">0,80%,1.0</Transform>
            </Map>
    
            <!-- (at this point, we should have a hill shaded area map) -->
    
            <!-- next up, the features -->
            <Group id="features">
                <!-- render the mapnik layer first -->
                <Map href="features.xml"/>
                <!-- then replace the alpha channel of the resulting buffer with that of
                 the rendered labels: -->
                <Channel src="A" dst="A">
                    <!-- render the labels into the buffer then blur, invert, and level -->
                    <Map href="labels.xml">
                        <Filter type="blur">0,2.0</Filter>
                        <Transform type="invert"/>
                        <Transform type="level">5,8%</Transform>
                    </Map>
                </Channel>
            </Group>
    
        <!-- finally, composite the non-shaded areas and labels -->
    
            <Map href="noshade-fills.xml"/>
            <Map href="labels.xml"/>
    
        </Composite>
```

The idea here is that the composition starts with an empty buffer and draws layers into it recursively, in the order that they appear in the XML. The `areas.xml` Mapnik stylesheet is rendered directly into the buffer, then the hill shading is applied twice: first darkened and composited using the "screen" blend mode; then lightened and composited using the "multiply" blend mode. The "features" group creates a new temporary buffer, into which the `features.xml` Mapnik stylesheet is rendered, then has its alpha channel ("A") replaced by that of the rendered `labels.xml` stylesheet (which, before being applied, is blurred, inverted, and leveled). The "areas" buffer is then composited onto the hill-shaded area map, followed by straight-up alpha composited `noshade-fill.xml` and `labels.xml` Mapnik styles.

Basically, each XML results in an image operation:

* `<Group>` and `<Layer>` operations create new rendering contexts, the results of which are composited onto the buffer of the parent context (optionally specifying a blend mode).
* `<Transform>` and `<Filter>` operations modify the buffer of the parent rendering context. The blur filter and color transforms in this example use [ImageMagick](http://www.imagemagick.org/script/command-line-options.php) semantics, but they could be expressed otherwise.
* `<Channel>` operations render their child layers or groups into a temporary buffer, one or more channels of which are then applied to the parent context (optionally specifying a bitwise operator?).

Recursively rendered Mapnik stylesheets should inherit their SRS from the `<Composite>` element, if provided.
