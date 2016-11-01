# ElseFilter

A Rule containing an `<ElseFilter />` is evaluated if and only if no other Rule in the current Style matched (either because of the ScaleDenominator or the Filter).

Mapnik evaluates all Rules in a Style sequentially from top to bottom, testing each Rule's Filter against the current Object. When all Rules have been evaluated but no Filter matched the current Object, the Rules containing an `<ElseFilter />` are evaluated.

## Usage
An `<ElseFilter />` is usually used when you want to draw _all_ Objects read from the Datasource but some of them Different. In this case you place as many Rules with [Filters](/wiki:Filter/) in your Style and add a last Rule with an `<ElseFilter />` that matches every Object not matched by the specific Rules. In such a scenario setting `filter-mode="first"` on the Style may be desired, so that always exactly one Rule from the Style gets evaluated.

## Example in XML
For each Object read from the Datasource Mapnik checks if the current ScaleDenominator matches the Min- and Max-Conditions of the first Rule and it the Object has an attribute "major" with a value of "1".
 * If the Object _passes_ these tests, the first Rule is evaluated painted to the map. The two other Rules are ignored in this case.
 * If the Object _misses_ these tests, the two Rules with an `<ElseFilter />` are evaluated according to their Min- and MaxScaleDenominator.

```xml
    <Style name="contours">
      <Rule>
        <Filter>[major] = 1</Filter>
        
        <MaxScaleDenominator>25000</MaxScaleDenominator>
        <MinScaleDenominator>100</MinScaleDenominator>
        
        <LineSymbolizer>
          <CssParameter name="stroke">#fb9b67</CssParameter>
        </LineSymbolizer>
      </Rule>
      
      <Rule>
        <ElseFilter/>
        
        <MaxScaleDenominator>25000</MaxScaleDenominator>
        <MinScaleDenominator>5000</MinScaleDenominator>
        
        <LineSymbolizer>
          <CssParameter name="stroke">#f45906</CssParameter>
        </LineSymbolizer>
      </Rule>
      
      <Rule>
        <ElseFilter/>
        
        <MaxScaleDenominator>5000</MaxScaleDenominator>
        <MinScaleDenominator>100</MinScaleDenominator>
        
        <LineSymbolizer>
          <CssParameter name="stroke">#f4062A</CssParameter>
        </LineSymbolizer>
      </Rule>
    </Style>
```

## Example in Python

```python
    rule = mapnik.Rule()
    rule.set_else(True) # instead of rule.filter(...)
```

## See also
 * The normal [Filter](https://github.com/mapnik/mapnik/wiki/Filter)
 * The (other) special [AlsoFilter](https://github.com/mapnik/mapnik/wiki/AlsoFilter)
