# International Text

Mapnik supports unicode text through ICU and has only very rudimentary support for Right-to-left (RTL) languages.

This page is to document and discuss improvements needed to unicode text handling.

## Current Tickets

 * [#364](https://github.com/mapnik/mapnik/issues/364) / [#404](https://github.com/mapnik/mapnik/issues/404) - RTL mirroring
 * [#1154](https://github.com/mapnik/mapnik/issues/1154) - Arabic font spacing
 * [#519](https://github.com/mapnik/mapnik/issues/519) - RTL spacing wrong with numbers (UBIDI_MIXED)
 * [#112](https://github.com/mapnik/mapnik/issues/112) - Indic RTL font shaping
 * [#189](https://github.com/mapnik/mapnik/issues/189) / [#409](https://github.com/mapnik/mapnik/issues/409) - RTL wrapping (line breaks)
  * from osm trac: http://trac.openstreetmap.org/ticket/2182 and http://trac.openstreetmap.org/ticket/1515
 * [#550](https://github.com/mapnik/mapnik/issues/550) - line-follow: perhaps made worse by unicode chars, perhaps not, needs closer look
 * [#558](https://github.com/mapnik/mapnik/issues/558) - Character spacing not correct for nepali text
 * [#582](https://github.com/mapnik/mapnik/issues/582) - TextSymbolizer bug with Armenian letters

## Resources

 * "State of Text Rendering": http://behdad.org/text/
 * [HarfBuzz](http://www.freedesktop.org/wiki/Software/HarfBuzz)
 * [pango library](http://www.pango.org/)
 * [liblinebreak](http://vimgadgets.sourceforge.net/liblinebreak/)
   * If used Pango would need to be an optional dependency because we don't want to have to depend on whole GLib/GTK stack.
 * http://shapecatcher.com/
 * Mozilla bug that has good testcases that could be harvested: https://bugzilla.mozilla.org/show_bug.cgi?id=721821