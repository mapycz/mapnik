# Installing Mapnik in Windows

Mapnik binaries can be installed and configured manually on windows. See http://mapnik.org/pages/downloads.html for the latest download and instructions on installing.

If you are interested in installing Mapnik on other operating systems see [[Mapnik-Installation]]

If you an interested in compiling Mapnik from source on windows see the windows scripts at https://github.com/mapnik/mapnik-packaging

## Supported Features

The Windows builds have a slightly less complete feature set than is possible from source builds. Full support is planned, and this table tracks the versions in which new features are added:

| *Mapnik Feature* |*0.5.0*  |  *0.5.1*  | *0.6.0* | *0.6.1* | *0.7.0* | *0.7.1* | *2.2.0* |
|:-----------------|---------|-----------|---------|---------|---------|---------|--------:|
| Cairo Rendering | -| -| - | -| -| - | *** |
| ICU Unicode Support| - | -|  ***  |  *** |  *** | ***  | *** |
| Python 2.5 Support|  ***   |  *** |  ***   |  *** |  *** | *** | - |
| Python 2.6 Support| -| -| - | -|  *** | *** | - |
| Python 2.7 Support| -| -| - | -|  *** | *** | *** |
| Libxml2 Parser Support| -| -| - | ***  |  *** | *** | *** |
| Shapefile Plugin|  ***|  *** |  ***  |  ***  |  *** | *** | *** |
| Raster Plugin |  ***|  *** |  ***  |  ***  |  *** | *** | *** |
| PostGIS Plugin   |  ***|  *** |  ***  |  ***  |  *** | *** | *** |
| GDAL Plugin  | - | -|  ***  |  ***  |  *** | *** | *** |
| OGR Plugin| - | -| - | ***  |  *** | *** | *** |
| SQLite Plugin | - | -|  ***  |  ***  |  *** | *** | *** |
| GeoJSON Plugin | - | -|  -  |  -  |  - | - | *** |
| CSVJSON Plugin | - | -|  -  |  -  |  - | - | *** |
| OSM Plugin| - | -| - | -| -| -  | -  |
| shapeindex.exe| - | ***   |  ***  |  ***  |  *** | *** | *** |
| pgsql2sqlite.exe  | - | - |  ***  |  ***  |  *** | *** | *** |

 * - : not available
 * ***: available

* Note: as of Mapnik 0.6.1 memory-mapped files are disabled in the Shapefile Plugin in windows builds (see #342)

## Overview

This Guide will walk you through installing Mapnik and then running a test script to generate the sample map below:

[[/images/demo.png]]

## Manual Instructions

### Step 1: Download the Mapnik binary

Get the latest Windows binaries at http://mapnik.org/pages/downloads.html

### Step 2: Unzip and place Mapnik

Place the unzipped folder into `C:\mapnik-v2.2.0\`
 
### Step 3: Set your system and/or users environment variables:

Do this at _Control Panel->System->Advanced->Environment Variables_

- Add `;C:\mapnik-v2.2.0\lib;` to the `PATH` variable.
- **Note:** The `;` characters are separators so add them before or after other entries that may be in your `PATH` variable. If you are only adding `C:\mapnik-v2.2.0\lib` (and your `PATH` has not been modified/added to yet then technically you can leave out the `;` until you add more entries.
- **Note:** you may also need to set your user path environment variable.
- **Note:** Adding `;C:\mapnik-v2.2.0\bin;` to the `PATH` variable is for the following commands:
 - mapnik-config, pgsql2sqlite, shapeindex, svg2png, upgrade_map_xml
- If the variable `PATH` is not already present, add it.
- Setting this correctly allows the Mapnik python bindings to find the `mapnik.dll`
- for PYTHON support add:
 - PYTHON 2.7:   `;C:\mapnik-v2.2.0\python\2.7\site-packages;` to the `PYTHONPATH` variable.
 - Setting this correctly allows Python to find the Mapnik python bindings when you do `>>> import mapnik`

### Step 4: Ensure PATH variables are correct

- Open a new console by running "cmd" to test settings.
- Type "path" to make sure your PATH contains `C:\mapnik-v2.2.0\lib`

### Step 5: Test python import

- Run `C:\Python27\python.exe`
- Then type at a python prompt:

```python
import mapnik
```

- If you get no error message, you made it!
- If you do get an error message, see *Troubleshooting* below
 
### Step 6: Test demo program

- Open explorer, go to `C:\mapnik-v2.2.0\demo\python`
- Double click `rundemo.py`
- You should see several demo.* files output.

### Step 7: Learn more

Head over to [[GettingStarted]] for your first tutorial on the Mapnik Python API.

## Trouble Shooting

### Mapnik DLL not found

You get an error like:

    Can't find mapnik.dll

Solution: make sure that you complete *Step 3.a* properly.


### Mapnik Python not found

Problem: When importing mapnik in python you get:

```python
    >>> import mapnik
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    ImportError: No module named mapnik
```

Solution: make sure that you have put the `site-packages` folder on your PYTHONPATH by completing *Step 3.b.*

Problem: When importing mapnik in python you get:

```
ImportError: DLL load failed: The specified procedure could not be found.
```

One possible reason is an `libxml2` conflict. See http://stackoverflow.com/questions/7576751/installing-mapnik-on-windows-xp-fails-with-the-message-importerror-dll-load-fa for details on how to workaround and follow https://github.com/mapnik/mapnik-packaging/issues/109.

### Unknown (Windows) Dependency not found

Problem: When importing mapnik in python you get:

```python
    >>> import mapnik
    [...snip...]
    from _mapnik import *
    ImportError: DLL load failed: This application has failed to start because the application configuration is incorrect. Reinstalling the application may fix this problem.
```

Solution:

 * You may need to install the 2010 Microsoft Visual C++ 2010 Redistributable Package (`vcredist.exe`) from the [Microsoft](http://www.microsoft.com/en-us/download/details.aspx?id=5555).
 * You can get more information on what is missing or incompatible by examining mapnik.dll with [Dependency Walker](http://www.dependencywalker.com/). On a 64bit system you must use the 32bit Version.