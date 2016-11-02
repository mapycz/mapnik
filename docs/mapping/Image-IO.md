Mapnik supports reading and writing a variety of image formats.

These formats can be used as:

 - Output formats for rendered map tiles
 - Input formats for symbols or icons for symbolizers like the [[MarkersSymbolizer]]
 - Input formats for the [[Raster]] datasource plugin. Note: if you are looking for what raster geodata formats are supported see also what the [[GDAL]] datasource plugin supports.

## Format strings

Mapnik functions like `mapnik.Image.save` in python or `mapnik::save_to_string` in C++ accept a string for various image formats. The string assumes a format like:

```
  format:key=value
```

So, for example to create a full color, 32 bit PNG with a custom compression level you could do:

```python
im = mapnik.Image(256,256)
im.save('file.png','png32:z=1')
```

Where `z` indicates the `zlib` encoding option and `1` indicates the value, in this case `Z_BEST_SPEED` or fast compression at the expense of larger size. See below for all PNG options. Also note that the above code creates a blank image - in a real world situation you would likely use `mapnik.render` to render map data to the image before saving it.

## Supported Formats

As of Mapnik 2.3.x the following formats are supported:

| Format | Read | Write  |
| -------| ---: | -----: |
| PNG    |  ✓   |   ✓   |
| JPEG   |  ✓   |   ✓   |
| TIFF   |  ✓   |   experimental   |
| WEBP   |  experimental   |   experimental   |

## Default output details

In Mapnik >= 2.3.x by default:

 - PNG output is full color png (aka. `png24` or `png32`)
 - JPEG output uses a `quality` of 85.
 - TIFF output uses `PHOTOMETRIC_RGB` and `COMPRESSION_DEFLATE`
 - WEBP output uses all WEBP encoder defaults meaning lossy compression (`lossless=0`) and a default quality of `75` (`quality=75`).

In Mapnik >= 3.x by default:

 - PNG output is paletted png with no greater than 256 colors (aka. `png8`)

See the C++ [image_util.cpp](https://github.com/mapnik/mapnik/blob/master/src/image_util.cpp) file for definitive defaults that are set for various encoders (as this wiki page may become out of date).

### TIFF output options

TIFF output support in Mapnik is experimental and does not yet support any user-configurable options. It also does not output geotiff tags. These are features we may support in the future if there is interest

### JPEG output options

JPEG output support in Mapnik is robust but simple. You can control just one option: the `quality` of the jpeg authored.

`quality` can be an integer value between `1` and `100` and can be passed to Mapnik by appending the number to the format string like:

 - `jpeg100` - will use a quality of `100`
 - `jpeg50` - will use a quality of `50`

### PNG output options

There generally two types of PNG Mapnik can author: 1) reduced color paletted png, and 2) full color png. Mapnik also accepts a variety of key:value options that can customize the encoding and may apply to only one kind of PNG type.

So, the two main types can be requested using the formats names:

| Name | Type |
| ---- | ----- |
| `png`, `png8` , or `png256` | Creates reduced color/quantized paletted png NOTE: in Mapnik versions older than 2.3.x the `png` keyword used to map to full color png|
| `png24` or `png32`     | Creates full color png with millions of possible colors (and much larger file size) |

And the key:value options can be controlled as follows:

| Key  | Value | Default | Description |
| ---- | ----- | ------- | ----------- |
| c    | integer, 0-256 | 256 | Max number of colors to allow in the image, the fewer colors the smaller the final size, but potential lower visual quality. It is not recommended to reduce this value below 64 unless your rendered map style is very simple because otherwise adjacent tiles might end up with different final colors for the same original color. |
| z    | integer, -1 to 9 | -1 (Z_DEFAULT_COMPRESSION) | Level of compression - directly maps to [zlib](http://www.zlib.net/) options:  0 is no compression, 1 is BEST_SPEED, and 9 is BEST_COMPRESSION (available in >= Mapnik 2.x) |
| t    | integer, 0 to 2| 2 | Transparency mode: 0-no alpha, 1-binary alpha(0 or 255), 2-full alpha range, default is 2, works differently depending on whether you have requested full color png or paletted png. In >= Mapnik 2.2.x `png32:t=0` will create full color png that is `rgb` and not `rgba` ([more details](https://github.com/mapnik/mapnik/issues/1559). If using paletted png then this option is most meaningful for the `octree` encoder (see `m=o` below).
| m    | string, `o` or `h` | `h` | Applies to paletted png only. This is the quantization method: `o` stands for `octree` and `h` stands for `hextree`. The `octree` quantizer only supports limited alpha ranges and so for images with detailed alpha this may produce a poorer quality image. However the `octree` encoder is faster than the `hextree` encoder. The `hextree` encoder is default (as of >= Mapnik 2.3.x) because it produces the highest quality output - nearly visually identical to full color png. If your maps do no contain any alpha (e.g. they have a completely opaque background) then full color png or `png8:m=o` may produce smaller pngs at a faster rate, however we plan to optimize this pathway automatically [in the future](https://github.com/mapnik/mapnik/issues/2029). |
| g    | float, 1.0 - 2.0 |  2.0 | Not likely that you need to change this option. It is the gamma correction for pixel arithmetic in hextree method. 1.0 means no gamma correction |
| s    | string, `default`, `filtered`, `huff`, or `rle` | `default` | Not likely that you need to change this option. It is the ZLIB compression strategy. See zlib docs for more details (available in Mapnik >= 2.x) |
| e    | string, `miniz` or `libpng` | 'libpng` | REMOVED in Mapnik 3.0.10 |

### WEBP output options

The WEBP API supports a lot of options and types of customization.

We support every option in the [Advanced Encoding API](https://developers.google.com/speed/webp/docs/api#advanced_encoding_api) except `show_compressed`. Below we provide basic descriptions for each option name pulled from the Advanced Encoding API documentation. Be aware that they are terse and you should also likely review the [cwebp encoder docs](https://developers.google.com/speed/webp/docs/cwebp) for more detailed descriptions of encoding options.

In addition to the webp core options that we expose we support one custom option called `alpha`. If you pass `webp:alpha=false` then an `rgb` webp image will be created instead of an `rgba` image. It is unclear yet whether there is any major benefit to encoding `rgb` webp images and so this option may be removed in the future. It is not recommended to use it. There is also some performance overhead to using this option along with image views (commonly used when metatiling) because, due to the nature of the webp API, [we have to make a copy of the image buffer in order to strip alpha](https://github.com/mapnik/mapnik/blob/e618a654985194a95f7cf9c8d48f82e2d1062057/include/mapnik/webp_io.hpp#L92-L113). At least this was the case when this functionality was developed against libwebp 1.3.1.

We default to all webp defaults. This means that the default encoding is `quality=75` and `lossless=0`.

The `image_hint` details (from the webp sources) are:

| value | meaning |
| ----- | ------- |
| 0 | default / `WEBP_HINT_DEFAULT` |
| 1 | picture / `WEBP_HINT_PICTURE` (digital picture, like portrait, inner shot) |
| 2 | photo / `WEBP_HINT_PHOTO` (outdoor photograph, with natural lighting) |
| 3 | graph / `WEBP_HINT_GRAPH` (Discrete tone image (graph, map-tile etc).) |

And all the `WebPConfig` based advanced options are:

```c++
struct WebPConfig {
  int lossless;           // Lossless encoding (0=lossy(default), 1=lossless).
  float quality;          // between 0 (smallest file) and 100 (biggest)
  int method;             // quality/speed trade-off (0=fast, 6=slower-better)

  WebPImageHint image_hint;  // Hint for image type (lossless only for now).

  // Parameters related to lossy compression only:
  int target_size;        // if non-zero, set the desired target size in bytes.
                          // Takes precedence over the 'compression' parameter.
  float target_PSNR;      // if non-zero, specifies the minimal distortion to
                          // try to achieve. Takes precedence over target_size.
  int segments;           // maximum number of segments to use, in [1..4]
  int sns_strength;       // Spatial Noise Shaping. 0=off, 100=maximum.
  int filter_strength;    // range: [0 = off .. 100 = strongest]
  int filter_sharpness;   // range: [0 = off .. 7 = least sharp]
  int filter_type;        // filtering type: 0 = simple, 1 = strong (only used
                          // if filter_strength > 0 or autofilter > 0)
  int autofilter;         // Auto adjust filter's strength [0 = off, 1 = on]
  int alpha_compression;  // Algorithm for encoding the alpha plane (0 = none,
                          // 1 = compressed with WebP lossless). Default is 1.
  int alpha_filtering;    // Predictive filtering method for alpha plane.
                          //  0: none, 1: fast, 2: best. Default if 1.
  int alpha_quality;      // Between 0 (smallest size) and 100 (lossless).
                          // Default is 100.
  int pass;               // number of entropy-analysis passes (in [1..10]).

  int show_compressed;    // if true, export the compressed picture back.
                          // In-loop filtering is not applied.
  int preprocessing;      // preprocessing filter (0=none, 1=segment-smooth)
  int partitions;         // log2(number of token partitions) in [0..3]
                          // Default is set to 0 for easier progressive decoding.
  int partition_limit;    // quality degradation allowed to fit the 512k limit on
                          // prediction modes coding (0: no degradation,
                          // 100: maximum possible degradation).
};
```