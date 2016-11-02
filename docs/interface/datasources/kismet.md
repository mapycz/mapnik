<!-- Name: Kismet -->
<!-- Version: 4 -->
<!-- Last-Modified: 2010/11/13 10:14:09 -->
<!-- Author: kunitoki -->


**WARNING: experimental !**

Mapnik's PluginArchitecture supports the use of different input formats.

This plugin supports reading WLANs found by running the [kismet](http://www.kismetwireless.net/) daemon. 


# Installation

There needs to be a running _kismet_server_ process with activated GPS support before starting mapnik. 


# Parameters

| *parameter*       | *value*  | *description* | *default* |
|:------------------|----------|---------------|----------:|
| host                  | string       | host where the kismet daemon is running | |
| port                  | integer      | port of the kismet daemon | |
| extent                | string       | max extent of the kismet returned wlans | |
| encoding              | string       | internal file encoding | utf-8 |


# Example

See next an example XML file to render WLAN icons on the map.

```xml
    <?xml version="1.0" encoding="utf-8"?>
    <!DOCTYPE Map>
    <Map bgcolor="#b5d0d0" srs="+proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over">
    
      <Style name="kismet">
        <Rule>
          <MaxScaleDenominator>2000000</MaxScaleDenominator>
          <MinScaleDenominator>100</MinScaleDenominator>
          <Filter>[internet_access]='wlan_crypted'</Filter>
          <PointSymbolizer file = "/home/andreas/src/osm/mapnik/symbols/wlan_crypted.png" type="png" width="32" height="32" />
          <TextSymbolizer name="name" face_name="DejaVu Sans Bold" size="8" fill="#636" dy="-10" halo_radius="1" wrap_width="0"/>
        </Rule>
    
        <Rule>
          <MaxScaleDenominator>2000000</MaxScaleDenominator>
          <MinScaleDenominator>100</MinScaleDenominator>
          <Filter>[internet_access]='wlan_uncrypted'</Filter>
          <PointSymbolizer file = "/home/andreas/src/osm/mapnik/symbols/wlan_uncrypted.png" type="png" width="32" height="32" />
          <TextSymbolizer name="name" face_name="DejaVu Sans Bold" size="8" fill="#636" dy="-10" halo_radius="1" wrap_width="0"/>
        </Rule>
    
        <Rule>
          <MaxScaleDenominator>2000000</MaxScaleDenominator>
          <MinScaleDenominator>100</MinScaleDenominator>
          <Filter>[internet_access]='wlan_wep'</Filter>
          <PointSymbolizer file = "/home/andreas/src/osm/mapnik/symbols/wlan_wep.png" type="png" width="32" height="32" />
          <TextSymbolizer name="name" face_name="DejaVu Sans Bold" size="8" fill="#636" dy="-10" halo_radius="1" wrap_width="0"/>
        </Rule>
      </Style>
    
    <!-- Layer -->
    
      <Layer name="kismet" status="on" srs="+proj=latlong +datum=WGS84">
        <StyleName>kismet</StyleName>
        <Datasource>
          <Parameter name="type">kismet</Parameter>
          <Parameter name="host">localhost</Parameter>
          <Parameter name="port">2501</Parameter>
          <Parameter name="estimate_extent">false</Parameter>
          <Parameter name="extent">-179,-85,179,85</Parameter>
        </Datasource>
      </Layer>
    </Map>
```

# Resources

This [icon](http://openclipart.org/people/pinterb7/pinterb7_wlan_accesspoint.svg) suits perfect for WLAN. I just changed the color and exported it to PNG.

Here is a video: http://www.youtube.com/watch?v=On9O8d7AOZA
