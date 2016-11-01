# Group symbolizer

Group symbolizer is used to place multiple road shields or other labels, grouped as single point placements on a feature. Multiple labels can be represented on a single feature using indexed column names. These indexed columns are matched to a single set of rules within the group symbolizer. This single set of style rules greatly simplifies the style definition needed to represent many different combinations of shields or labels. For example, if your shield styles use the data columns "type" and "number", you can provide columns "type1", "type2", "number1", and "number2". You can then reference these in the style rules and symbolizers as [type%] and [number%], such that both pairs of values will be matched separately to create two different shields. The individual labels in the group get positioned automatically by a group layout defined within the symbolizer.

See [examples](#wiki-examples) below.

## Configuration Options for GroupSymbolizer
### Data column options
| *parameter* | *type*  | *description* | *default* |
|----------------|---------|----------------|------------|
|start-column|integer|Starting index for indexed column names.|1|
|num-columns|integer|Number of labels that will be represented by indexed columns. Indexed columns will range from start-column to start-column + num-columns|0|
|repeat-key|expression|Pattern to create unique key from data columns to indicate identical labels. This is used with minimum-distance to avoid excessive repetition of identical labels across features. - Note: This value can be overridden in the individual group rules.||

### Placement options
| *parameter* | *values / type*  | *description* | *unit* | *default* |
|----------------|---------|----------------|-------|------------|
|placement|line, point, vertex, interior|"line" to label along lines instead of by point. TODO: Document other options.||point|
|spacing|double|Space between repeated labels. If spacing is 0 only one label is placed.|px|0|
|minimum-distance|double|Minimum distance between repeated label groups, as defined by repeat-key (works across features)|px|0.0|
|label-position-tolerance|double|Allow labels to be moved from their point in line placement. Lower values indicate that Mapnik tries less positions and generally leads to fewer labels. Higher values lead to Mapnik trying more different positions along a line to find a free spot. If unset or 0, Mapnik sets this value based on the total length of the line to ensure enough labels are placed.|?|0|
|allow-overlap|true, false|Allow labels to overlap other labels - Note: you can also clear the label collision cache at the LAYER level to promote more overlap. See 'clear-label-cache' at [[XMLConfigReference]] part layer|bool|false|
|avoid-edges|true, false|Avoid placing labels that extend over the edges of the map|bool|false|
|minimum-padding|double|Minimum distance that label must be from edges of the map in order to be placed|px|0.0|
|clip|true, false|If true then the geometry is clipped to the view before doing placements. Improves performance but can cause bad placements when the results are used for tiling|bool|true|
|minimum-path-length|double|place labels only on paths longer than this value.|px|0.0|
|force-odd-labels|true, false|Force an odd amount of labels to be generated.|bool|false|
|largest_bbox_only|true, false|Controls default labeling behavior on multipolygons. The default is `true` and means that only the largest polygon part is labeled. NOTE: this option may change or be renamed in the future|bool|true|2.1|

##Children of GroupSymbolizer

###Layout

The layout defines how the group of shields or labels should be arranged. There are two types available: SimpleLayout and PairLayout. Only one layout should be defined inside of a group symbolizer.

####SimpleLayout
Simple layout arranges the labels in a horizontal row, centered on the placement point.

####PairLayout
Pair layout arranges the labels in horizontal pairs. The first is aligned left of the placement point, and the second is aligned right. If more than two labels are present, the pairs stack vertically above and below the placement point.

###GroupRule

A group rule is defined very similar to a regular style rule. It contains a filter and any number of symbolizers. A group rule can also override the repeat key defined at the group symbolizer level. When the repeat key is defined on the group rule, any label matching this rule will use the rule's repeat key.

## Examples

Simple XML Example:
```xml
<GroupSymbolizer num-columns="2">
    <SimpleLayout />
    <GroupRule>
        <Filter>[ref%].match('US ')</Filter>
        <PointSymbolizer file="us-shield.png"/>
        <TextSymbolizer>[num%]</TextSymbolizer>
    </GroupRule>
    <GroupRule>
        <Filter>[ref%].match('I ')</Filter>
        <PointSymbolizer file="interstate-shield.png"/>
        <TextSymbolizer>[num%]</TextSymbolizer>
    </GroupRule>
</GroupSymbolizer>
```
