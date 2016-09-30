
# Layer level compositing

Layers support `comp-op` and `opacity` properties. If one of these properties set, a new rendering buffer is associated with the layer and rendering goes to that buffer. When rendering of the layer and its sublayers is done, the associated buffer is composited into parent buffer (map or parent layer).

If neither `comp-op` nor `opacity` explicitly defined, rendering buffer is inherited from parent layer.

## Examples

See following visual tests.

### comp-op

[nested-layers-1.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/nested-layers-1.xml)

![nested-layers-1](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/nested-layers-1-512-256-1.0-agg-reference.png)

### opacity

[layer-opacity.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/layer-opacity.xml)

![layer-opacity](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/layer-opacity-512-512-1.0-agg-reference.png)
