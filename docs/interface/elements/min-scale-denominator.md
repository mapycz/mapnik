# MinScaleDenominator

Activates the rule, if [ScaleAndPpi scale] >= minScaleDenominator - 1e-6

Example:

```xml
    <Style name="text">
      <Rule>
        <Filter>[place] = 'city'</Filter>
        <MaxScaleDenominator>10000000</MaxScaleDenominator>
        <MinScaleDenominator>2000000</MinScaleDenominator>
        <TextSymbolizer name="name" face_name="DejaVu Sans Book" size="10" fill="#000" dy="0" halo_radius="1" wrap_width="0"/>
      </Rule>
    </Style>
```

```python
    thin_lines_rule = Rule()
    thin_lines_rule.min_scale = 2500001
    thin_lines_rule.max_scale = 5000000000000
    thin_state_line = LineSymbolizer(Color('#A0A0AA'),.5)
    thin_lines_rule.symbols.append(thin_state_line)
    state_borders.rules.append( thin_lines_rule )
```

It appears that both min and max scale denominators are required. Also, note that if you are writing multiple rules which format items different on x > y > z, reformat to rules which format on x > y + 1, y > z
