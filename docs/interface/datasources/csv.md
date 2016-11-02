# CSV plugin

This plugin can read tabular data with embedded geometries. It auto-detects column types based on common sense type coercion and auto-detects geometry data by looking for header names like `latitude/lat` and `lon/long/lng/longitude` that likely encode point data. It can also handle GeoJSON and WKT encoded geometries and will find these if any column headers are named `geojson` or `wkt` (case-insensitive).

It supports reading CSV data from files if you pass the `file` option. But if you instead pass the `inline` option and provide CSV data as a string then it will read this data instead of looking on the filesystem. An example of this usage is describe in the page on [how to author good testcases](https://github.com/mapnik/mapnik/wiki/A-perfect-testcase#portable).

This plugin reads the entire file upon initialization and caches features in memory so it is extremely fast for rendering from after initial startup (for reasonable size files under 5-10 MB).

For more details on the motivations and design of this plugin see: https://github.com/mapnik/mapnik/issues/902

Details on auto-detection:

### headers
The plugin requires headers be present that give each column a name. It will parse the first line of the file as headers. If the first line is not actually headers then parsing behavior will be unpredictable because the plugin makes no attempt to detect whether the first line is pure data. You should ideally edit your CSV, adding headers if it does not have them. For cases where the CSV file may be very large or it is not feasible to add headers, then the plugin can be asked to dynamically accept them. Simply pass the option called `headers`, providing a list of names separated by your delimiter of choice like: `headers=x,y,name`. This would work for a csv file lacking headers like:

```csv
-122,48,Seattle
0,51,London
```

Or `headers=x|y|name` if your data is `|` delimited.

### file size

Because the plugin caches data in memory, large file sizes are not recommended. If your data is large, then please choose a more suitable format and then Mapnik can easily read data one row at a time. Because the CSV Plugin reads all data at initialization it makes an attempt to detect files over a certain size and will throw an error. The default threshold is 20 MB. This can be changed by passing the `filesize_max` option an alternative MB amount, but doing so is not recommended unless you are sure your machine has sufficient memory.

### line breaks

The plugin will read the first 2000 bytes of the file to count the occurrences of `\n` and `\r`. Which ever is more plentiful will be the assumed character that signifies line breaks - this allows the plugin to automatically work for both mac, windows, and unix style line breaks.
 
### column separator

The plugin will read the first line of the file to count the occurrences of `,`, `\t`, `|`, and `;` as possible delimiters of columns. You can disable this auto-detection by passing the `separator` option.

### escape character

The plugin defaults to assuming `\` is the escape character. So data like `"This is a cell\"s data"` would parse well. It has spaces so it needs to be quoted and while it uses the quoting character in the text it is properly escaped. If you prefer to use a different escape character then pass the `escape` option.

### quote character

The plugin defaults to assuming `"` is the quote character that lines needing quoted will be wrapped in. Therefore a line like `"Main street,USA"` would parse fine and the `,` inside would not be interpreted as a column separator, but `'Main street, USA'` would not parse unless you passed a custom option like `quote='`.
