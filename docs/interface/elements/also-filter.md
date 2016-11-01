# AlsoFilter

A Rule containing an `<AlsoFilter />` is evaluated if and only if at least one other Rule in the current Style matched (according to its ScaleDenominator and Filter).

Mapnik evaluates all Rules in a Style sequentially from top to bottom, testing each Rule's Filter against the current Object. When all Rules have been evaluated and at least one Filter matched the current Object, the Rules containing an `<AlsoFilter />` are evaluated.

## Usage
An `<AlsoFilter />` is useful when you want to draw a bunch of similar things (like POI-Icons) and you would want to add another Symbolizer to each and every Rule in the style. Using a separate `<AlsoFilter />`-Rule avoids duplicating the other Symbolizer over and over.

Please note, that there are different methods of creating such combined styles, each having their own pros and cons. One possibility would be using [[ShieldSymbolizer]]s, which would keep the Icon and the Text together and would never lead to an icon without text or vice versa. In complex styles it's sometimes necessary to place the [[PointSymbolizer]]s into a different Style then the [[TextSymbolizer]]s in order to gain greater control about the drawing order.

## Behavior regarding filter-mode="first"
With #706 the new `filter-mode` attribute was introduced to the Style object. Setting `filter-mode="first"` in conjunction with an `<AlsoFilter />`-Rule will apply only the first matching Rule of the Style. If a Rule matched, all `<AlsoFilter />`-Rules are applied, too. If no Rule matched, all `<ElseFilter />`-Rules are evaluated.

## Example in XML
For each Object read from the Datasource Mapnik checks if the current ScaleDenominator matches the Min- and Max-Conditions of the first Rule and it the Object has an attribute "amenity" with a value of "restaurant" or "pub".
 * If the Object _passes_ these tests, the according rule is evaluated. After all normal Rules have been evaluated, the Rules containing an `<AlsoFilter />` are also evaluated.
 * If the Object _misses_ these tests, none of the rules is evaluated.

```xml
    <Style name="poi-food">
      <Rule>
        <Filter>[amenity] = 'restaurant'</Filter>

        <MaxScaleDenominator>25000</MaxScaleDenominator>
        <MinScaleDenominator>100</MinScaleDenominator>

        <PointSymbolizer file="restaurant.png" />
      </Rule>
      <Rule>
      <Filter>[amenity] = 'pub'</Filter>

        <MaxScaleDenominator>25000</MaxScaleDenominator>
        <MinScaleDenominator>100</MinScaleDenominator>

        <PointSymbolizer file="pub.png" />
      </Rule>

      <Rule>
        <AlsoFilter />

        <TextSymbolizer name="[name]" fontset-name="book-fonts" size="9" fill="black" dy="9" halo-radius="1" wrap-width="0" />
      </Rule>
    </Style>
```

## Example in Python

```python
    rule = mapnik.Rule()
    rule.set_also(True) # instead of rule.filter(...)
```

## See also
 * The normal [Filter](https://github.com/mapnik/mapnik/wiki/Filter)
 * The (other) special [ElseFilter](https://github.com/mapnik/mapnik/wiki/ElseFilter)
