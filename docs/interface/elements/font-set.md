<!-- Name: FontSet -->
<!-- Version: 1 -->
<!-- Last-Modified: 2009/01/28 20:44:31 -->
<!-- Author: Beau Gunderson -->
The FontSet element defines a group of fonts to be used in cases where it is desirable to support more characters than exist in a single font (also referred to as "fallback fonts" support).

For example, on a layer with both English and Chinese names, one could specify fonts with a high level of legibility for English letters as well as a fallback font for Chinese characters that don't exist in the English font.

Here's a small example:

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <!DOCTYPE Map>
    <Map srs="+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs">
     <FontSet name="my-fonts">
      <Font face-name="DejaVu Sans Book" />
      <Font face-name="TSC FMing S TT Regular" />
     </FontSet>
    
     <Style name="font-test">
      <Rule>
       <TextSymbolizer name="NAME" fontset-name="my-fonts" size="15" fill="black" />
      </Rule>
     </Style>
    
     <Layer name="font-test" status="on" srs="+proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs">
      <StyleName>font-test</StyleName>
       <Datasource>
        <Parameter name="type">shape</Parameter>
        <Parameter name="file">Font_Test</Parameter>
       </Datasource>
      </Layer>
    </Map>
```