# Using Custom Fonts with Mapnik

Doing this requires three steps:

## 1) Place custom fonts in Mapnik's font folder or register them in different location

### Python
Find the location of Mapnik's special font folder and place new fonts in it:


```sh
    $ python -c "import mapnik;print mapnik.fontscollectionpath"
```

Or register a custom location:

```python
    import mapnik
    custom_fonts_dir = '/Library/Fonts/'
    mapnik.register_fonts(custom_fonts_dir)
```

### XML
We are planning to add registration support within XML in [#168](https://github.com/mapnik/mapnik/issues/168)

  * Also, the [[Nik2Img]] allows you to pass the `--fonts` argument to register custom fonts when reading and XML

### C++

```cpp
    mapnik::freetype_engine::register_font(std::string const& file_name)
```

...where file_name is the full path to the directory where the font is located (e.g., "/home/user/mapnik/fonts/unifont-Medium.ttf"). The c++ demo (source:trunk/demo/c++/rundemo) also provides an good example of how to add additional fonts.

## 2) Figure out the exact 'face_name'

  * Ask Mapnik to print all registered face names (*note*: this only will show fonts already auto-registered in the `fontscollectionpath`):

```sh
    python -c "from mapnik import FontEngine as e;print '\n'.join(e.instance().face_names())"
    DejaVu Sans Bold
    DejaVu Sans Bold Oblique
    DejaVu Sans Book
    DejaVu Sans Condensed
    DejaVu Sans Condensed Bold
    [..snip..]
```

 * Or, register custom fonts and then show their face names:

```python
    from mapnik import register_fonts, FontEngine
    custom_fonts_dir = '/Library/Fonts/'
    register_fonts(custom_fonts_dir)
    for face in FontEngine.face_names(): print face
    ... 
    Al Bayan Bold
    Al Bayan Plain
    Andale Mono Regular
    Arial Black Regular
    [...snip...]
```

## 3) Pass the 'face_name' to the TextSymbolizer or ShieldSymbolizer


```xml
    <TextSymbolizer name="FIELD_NAME" face_name="DejaVu Sans Condensed Bold" size="10" fill="black" />
```


## Further references

 * Nice Summary at: http://weait.com/content/add-fonts-your-openstreetmap-server
 * `load_fonts()` recursive C++ function in https://github.com/openstreetmap/mod_tile/blob/master/src/gen_tile.cpp#L138-L170
