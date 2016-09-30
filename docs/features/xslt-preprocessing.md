
# XSLT preprocessing

To maximally leverage the XML interface, an XML style is automatically transformed if an XSLT found. XSLT style can be either embedded into XML style or linked to a standalone file.

## Examples

### Embedded

```xml
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet type="text/xsl" href="#my_style"?>
<!DOCTYPE Map [
<!ATTLIST xsl:stylesheet id ID #REQUIRED>
]>
<Map>
    <xsl:stylesheet id="my_style" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
        <xsl:template name="discard_xsl" match="xsl:*">
        </xsl:template>

        <xsl:template match="@*|node()">
            <xsl:copy>
                <xsl:apply-templates select="@*|node()"/>
            </xsl:copy>
        </xsl:template>
    </xsl:stylesheet>

    <Style name="style">
    </Style>
    <Layer name="layer">
        <StyleName>style</StyleName>
    </Layer>
</Map>

```

### Standalone XSL

```xml
<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet id="my_style" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
    <xsl:template match="@*|node()">
        <xsl:copy>
            <xsl:apply-templates select="@*|node()"/>
        </xsl:copy>
    </xsl:template>
</xsl:stylesheet>
```

```xml
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet type="text/xsl" href="style.xsl"?>
<Map>
    <Style name="style">
    </Style>
    <Layer name="layer">
        <StyleName>style</StyleName>
    </Layer>
</Map>
```

### Visual test

Following visual test shows more complicated XSLT, including [dynamic evaluation](http://exslt.org/dyn/index.html).

[xslt-preprocess.xml](https://github.com/mapycz/test-data-visual/blob/master/styles/xslt-preprocess.xml)

![xslt-preprocess](https://github.com/mapycz/test-data-visual/blob/master/images/xslt-preprocess-600-400-1.0-agg-reference.png)
