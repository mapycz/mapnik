# Keyed collision caches

All symbolizers working with collision detector have parameters `collision-cache-detect` and `collision-cache-insert`. These parameters set names of collision detectors. Multiple names delimited by comma are supported. If one of these parameters is not defined, collision detector with name `default` is used instead. The `default` collision detector can be referenced explicitly.

An example of inserting objects only into collision detector called `water`:

```
collision-cache-insert="water"
```

An example of detection against collision detector called `water` and default collision detector:

```
collision-cache-detect="water,default"
```

## `DebugSymbolizer`

`DebugSymbolizer` has parameter `collision-cache` for comma separated list of collision detectors to visualize. If `collision-cache` is not set, all objects of all collision detectors are visualized.

## An example

[collision-cache-1.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/collision-cache-1.xml)

![collision-cache-1](https://raw.githubusercontent.com/mapycz/test-data-visual/master/images/collision-cache-1-800-800-1.0-agg-reference.png)
