http://www.slideshare.net/steve8/what-id-like-to-do-with-mapnik-4756837

1) icon rotation: could also be used for amenity=viewpoint when tagged with viewing angles

2) <strike>SVG icons: allow replacing colors (e.g. color a black icon in blue); have an alpha parameter</strike> (done)

3) move icon a bit if it would collide with others

4) replace an already placed icon with a collection icon when there is another one to be shown right there
 - unlikely to implement this, as it is better done as a (zoom dependent) pre-processing step before mapnik

5) <strike>push lines apart so that there is a minimum distance between them</strike> (nearly done)

6) <strike>be able to construct file names from tag values in XML (e.g. "symbol_file=parking.svg" should be mapped to e.g. ~/mapnik/parking.svg or http://example.com/parking.svg). Right now, people resort  to generating XML styles by a script.</strike> (done, see "PathExpressions")

7) when there is a very curvy road, place the name along a smoothed line (when the text is not centered to the line but with a dy) (nearly done)

8) <strike>multi-line text constructed from several tags, e.g. for peak name and height (can be done when you accept the same styling for these text fragments)</strike>

9) stronger coupling between icon and text (maybe similar to the ShieldSymbolizer?) so that manual text dy tweaking is not required anymore (want to be able to say: "place this icon in size 20x20  and put the text in size 12pt five pixels below it") Idea: “TownSymbolizer”.

10) collapsing of little small areas with the same tags into a single big one (for e.g. a big forest), so that e.g. the name is drawn only once
 - unlikely to implement this, as it is better done as a (zoom dependent) pre-processing step before mapnik

11) be able to suppress automatic text rotation (per style or layer) - contour lines should always be displayed so that the base line of the text points to the lower ground

12) underlined text (for e.g. capitals)

13) sometimes, peak names drawn in a half circle, would be cool if Mapnik could do that as well

## Labeling

http://pal.heig-vd.ch
