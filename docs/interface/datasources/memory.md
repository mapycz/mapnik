# MemoryDatasource

A in-memory datasource. Sample usage:

```python
    import mapnik
    ds = mapnik.MemoryDatasource()
    f = mapnik.Feature(mapnik.Context(), 1)
    f.add_geometries_from_wkt("POINT(2 5)")
    ds.add_feature(f)
```
