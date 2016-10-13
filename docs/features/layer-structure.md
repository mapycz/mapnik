
# Tree-like layer structure

Layers can be arbitrarily nested, in other words layers can be made of another layers. This allows better layer organization and complex grouped compositing, see [Layer level compositing](layer-level-compositing.md).

## Rendering rules

Layers are rendered in top-down order as they are defined. Rendering and even querying data by a layer is not proceeded in following situations:

* A layer is deactivated (by `status` property in XML).
* A layer has no datasource defined.
* A layer is not visible for given scale denominator.
* Layer's extent doesn't intersect with map's extent.
* A layer has zero active styles.

If one of these conditions holds, all sub-layers are ignored.

## The spatial reference system definition (`srs`)

If the `srs` of a layer is not defined explicitly, it is inherited from parent layer or from map in case of top-level layer.

## An example

```xml
<Map>
    <Layer name="terrain">
        <StyleName>terrain</StyleName>
        <Datasource>
            ... some raster data ...
        </Datasource>

        <Layer name="contours">
            <StyleName>contours</StyleName>
            <Datasource>
                ... some vector data ...
            </Datasource>
        </Layer>
    </Layer>

    <Layer name="paths">
        ...

        <Layer name="level2">
            ...
        </Layer>
        <Layer name="level3">
            ...
        </Layer>
    </Layer>

    <Layer name="poi" status="off">
        ...

        <Layer name="poi2">
            ...
        </Layer>
        <Layer name="poi3">
            ...
        </Layer>
    </Layer>
</Map>

```


