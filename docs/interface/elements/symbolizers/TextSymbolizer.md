# TextSymbolizer

### Text layout options
| *parameter* | *values/ type*  | *description* | *unit* | *default* | *version* |
----------------|---------|----------------|-------|------------|---------|
|dx, dy|double|Displace label by fixed amount on either axis. Actual displacement also depends on vertical-alignment and horizontal-alignment|px|0.0|0.7
|vertical-alignment|top, middle, bottom, auto|Position of label relative to point position. "auto" is "middle" for dy=0, "bottom" for dy>0, "top" for dy<0||auto|
|horizontal-alignment|left, middle, right, auto|Position of label relative to point position||auto|
|justify-alignment|left, center, right, auto|Justify multi-line text||auto|
|text-ratio|double| Try to keep a given height to width ratio. Use 0 to disable this feature.|ratio|0|
|wrap-width|double|Length before wrapping long names. Use 0 to disable this feature.|px|0|
|wrap-before|true,false|Wrap text before wrap-width is reached. If this setting is off your lines will always be a bit longer than wrap-width. If this setting is on the lines will usually be a bit shorter, but can be longer if there is a single word that is longer than your current line limit.|bool|false|
|orientation|double|Rotate text.|degree|0|
|rotate-displacement|true, false|Rotates the displacement around the placement origin by the angle given by "orientation".|bool|false|harfbuzz|

### Character formatting options
| *parameter* | *values/ type*  | *description* | *unit* | *default* | *version* |
----------------|---------|----------------|-------|------------|---------|
|face-name|string| Font name (see [[UsingCustomFonts]])| | |
|fontset-name|string| FontSet name ||
|size|double|Font size|px|10.0|
|fill|color|Color of the text fill, e.g. #FFFFFF||black|
|halo-fill|color|Color of the text halo, e.g. rgba(255,255,255,0.5)||white|
|halo-radius|double|Radius of the halo in pixels (Fractional pixels are accepted. See halo-rasterizer for limitations.)|px|0|
|halo-comp-op|string|Composition only with the halo|||
|character-spacing|double|Additional horizontal spacing between characters. Currently works for point placement only, not line placement. You will get the normal spacing defined by the font plus this amount of extra space. (Also works for line placements in harfbuzz branch.)|px|0|
|line-spacing|double|Vertical spacing between lines of multi-line labels. This spacing is in addition to the normal font line spacing|px|0|
|wrap-character|char|Use this character instead of a space to wrap long names. (Harfbuzz branch ignores this and uses Unicode rules for line breaks.)|||
|text-transform|none, uppercase, lowercase, capitalize|Allows conversion of text to lower or upper case before display.||none|

## Examples

Some examples of Mapnik's ability to place text along lines:

[[/images/output_old.png]]

### XML


```xml
<TextSymbolizer face-name="DejaVu Sans Book" size="10" fill="black" halo-fill= "white" halo-radius="1" placement="line" allow-overlap="false">[FIELD_NAME]</TextSymbolizer>
```

See [[XMLGettingStarted]] for more XML example uses of TextSymbolizer.

### Python


```python
t = TextSymbolizer(Expression('[FIELD_NAME]'), 'DejaVu Sans Book', 10, Color('black'))
t.halo_fill = Color('white')
t.halo_radius = 1
t.label_placement = label_placement.LINE_PLACEMENT # POINT_PLACEMENT is default
dir(t) # for the rest of the attributes
```

### C++


```cpp
#include <mapnik/map.hpp>
#include <mapnik/font_engine_freetype.hpp>

using namespace mapnik;
try {
   freetype_engine::register_font("/path/to/font.ttf");
   /* some code */
   rule_type rule;
   text_symbolizer ts("[FIELD_NAME]", "DejaVu Sans Book", 10, color(0, 0, 0));
   ts.set_halo_fill(color(255, 255, 200));
   ts.set_halo_radius(1);
   rule.append(ts);
}
```

The first parameter is the field name of a database field, or from a shape file, or an OSM file. In case of a shape file or OSM file, the field name is case sensitive.
You must load the needed fonts first, otherwise you'll get a run time error. But you can load as many true type fonts as you like. Mapnik is coming with a couple of fonts in "mapnik/fonts". I recommend to load all of this fonts, regardless if you need them or not. 

## Placements
In Mapnik 2 the possibility to try different placements if the text can't be placed at the intended position is introduced. 

Algorithms:
### Simple
(This is the only algorithm supported in Mapnik 2.0)
It expects a string to specify which positions and size should be used. The format is POSITIONS,[SIZES].
POSITIONS is any combination of N, E, S, W, NE, SE, NW, SW (direction) and X (exact position as give by "displacement") (separated by commas, may not be empty).

[SIZE] is a optional list of font sizes, separated by commas. The first font size is always the one given in the TextSymbolizer parameters.

First all directions are tried, then font size is reduced and all directions are tried again. The process ends when a placement is found or the last font size is tried without success.

For this algorithm horizontal-alignment and vertical-alignment should be set to "auto".

Examples: 

* "N,S,15,10,8" (tries placement above with font-size give in "size" tag, then below and if that fails it tries placement above with size 15, then blow with size 15, above with 10, ...).
* "N,S" (only font size from "size" tag)
* "X,10,5" (keep position, but try to reduce size)
* _Invalid:_ "10,5" (no position specified)

Note: Whitespace is ignored, e.g. "N,S,15,10" and "N, S,15,10" and "N, S, 15, 10" are equivalent.

An XML example might look like:

```xml
<TextSymbolizer 
  allow-overlap="false"
  face-name="DejaVu Sans Book"
  placement-type="simple"
  placements="N,S,15,10,8"
>[label]</TextSymbolizer>
```

### List
(Supported starting with Mapnik 2.1)
Here a list of styles is defined and tried one by one till a valid position is found. Each style inherits from the previous one.

It is defined in XML by:
```xml
<TextSymbolizer face-name="DejaVu Sans Book" size="16" placement="point" dy="8" fill="blue" placement-type="list">[name]
    <Placement size="10" dy="-8" fill="red"/><!-- Reduces text size and changes offset -->
    <Placement fill="green">[abbreviated_name]</Placement> <!-- size="10", dy="-8", fill="green", shorter text -->
    <Placement fill="orange" dy="8">[nr]</Placement> <!-- size="10", dy="8", fill="orange", shortest text -->
</TextSymbolizer>
```
(Note [abbreviated_name] and [nr] have to be supplied by the data source!)

## New syntax
Starting with Mapnik 2.0 a new syntax is used:

```xml
<TextSymbolizer name="[label]" />
```

becomes

```xml
<TextSymbolizer>[label]</TextSymbolizer>
```

This change was made to be forward compatible with changes to text formatting being introduced in later versions.

## Alternate texts for upside down rendering
If some of your text depends on the line direction you need to supply different texts for each direction. This can be done like this:
```xml
<TextSymbolizer face-name="DejaVu Sans Book" size="10" 
   placement="line" upright="left_only" placement-type="list">
       "left only &lt;--"
       <Placement upright="right_only">"right only --&gt;"</Placement>
</TextSymbolizer>
```

## New features in Mapnik 3.x
* upright="auto/left/right" (See table above)
* dx is also used for line placements
* Multi-line support for line placements
* New parameter ``rotate-displacement="true/false"``
* Parameters now supported for `line` placement (that only previously supported for `point` placement): justify-alignment, vertical-alignment, text-ratio, wrap-width, wrap-before
* Line offsets are real offsets instead of fake offsets imitated by calculating the average angle
* If dy!="0" you have to set vertical-alignment="middle" to get the same behavior as in previous versions.

## Layouts

Additional text layouts can be defined in the contents of text symbolizer. Each of these can be assigned any of the text layout attributes listed above. Rather than being alternate placements, each layout is drawn as part of a single placement. This means that all of the text layouts must fit in order for any of them to be drawn. Different configurations of layout nodes can be specified for each alternate placement. Multiple text layouts are compatible with point and line placements.

A simple XML example:

```xml
<TextSymbolizer face-name="DejaVu Sans Book" size="12" placement="line">
    <Layout dy="-5">[LEFT_NAME]</Layout>
    <Layout dy="5">[RIGHT_NAME]</Layout>
</TextSymbolizer>
```

The above specifies two offset text layouts to be placed along opposite sides of a line.

## Formats
When you want to change attributes within the text symbolizer, you can use the ``<Format>`` element.
Example
```xml
<TextSymbolizer placement="point" size="20" face-name="DejaVu Sans Book" fill="black" justify-alignment="center">
    [name] + "\n" <Format size="10">"(" + [ele] + ")"</Format>
</TextSymbolizer>
```
This could result in this:

![](https://cloud.githubusercontent.com/assets/3618939/10868868/bc94ced6-809c-11e5-9b9d-36c587a4378c.png)

(See [#3158](https://github.com/mapnik/mapnik/issues/3158))
