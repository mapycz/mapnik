# Using SCons with Mapnik

Mapnik uses a python-based build system called [SCons](http://www.scons.org/wiki/TheBigPicture) to configure, build, and install Mapnik from source.

The build system is composed of a single `SConstruct` file in the main source directory and various `SConscript` files in subdirectories.

For more details see on SCons see:

* SCons documentation: http://www.scons.org/documentation.php
* Good Overview: http://www.humanized.com/presentations/scons/

## Quickstart

For the impatient just do:

```sh
    $ cd mapnik_src_dir
    $ python scons/scons.py configure # will configure compilation and save out configuration to a python pickle
    $ python scons/scons.py # will compile mapnik sources (running configure first if not done yet)
    $ sudo python scons/scons.py install # will install Mapnik (running configure and compiling first if not done yet)
```

## Common Questions

### Do I need to install SCons?

* No, for convenience Mapnik bundles a `scons-local` directory that includes all the SCons python modules needed to build Mapnik..
* This allows you to run scons locally within the Mapnik source folder like:

```sh
    $ python scons/scons.py
```

### I already have SCons installed, can I use it?

* Yes, to use your installed version of SCons rather than the bundled version just run the command:

```sh
    $ scons
```

### How does SCons compare to `make`?
* SCons is less verbose, handles dependency checking more thoroughly, and caches configuration settings differently.
* However, the basic user experience is quite similar to the common `make` commands:


|**Tool**|**Configure Step**|**Compile Step**|**Install Step**|
|:-------|------------------|----------------|---------------:|
|make|`$ ./configure`|`$ make`|`$ sudo make install`|
|SCons|`$ scons configure`|`$ scons`|`$ sudo scons install`|


For more details on SCons vs other build tools see: http://www.scons.org/wiki/SconsVsOtherBuildTools

---

# Old / outdated information


## Summary of  Changes between 0.5.x and 0.6.0

 * SCons now has an optional configure step which will configure your build, then exit.
  * To configure run $ python scons/scons.py configure SOMEVARIABLE=somevalue
  * Or simply specify SOMEVARIABLE=somevalue, however this will not exit and will proceed to compilation.

 * Any custom options you specify SCons will notice and save in a 'config.py' python file in the mapnik source directory.

 * Later custom options can be added to this 'config.py' file and will be re-read by SCons when `python scons/scons.py configure` is run again.
 
 * You can ause the CONFIG variable to point to one more files to load custom options from.

 * BOOST_TOOLKIT, BOOST_VERSION, and BOOST_ABI have been added to help point SCons at custom library names for boost.
  * BOOST_TOOLKIT is the name of the compiler uses to build boost.
  * BOOST_VERSION is the library name in the format 'major_minor' or '1_38' (note underscore).
  * BOOST_ABI is a single letter tag denoting the library's interoperability.
  * For more information see: http://www.boost.org/doc/libs/1_38_0/more/getting_started/unix-variants.html#library-naming

 * Cairo rendering is new and requires extra dependecies.
  * Cairo dependecies will be auto-detected using pkg-config
  * If you don't wish to have SCons attempt to find and build Cairo specify CAIRO=False

 * Postgres is now configured using the `pg_config` program and not the PGSQL_LIBS and PGSQL_INCLUDES variables.
  * If `pg_config` is not found use the PG_CONFIG variable to point to the full path of the program.

 * libxml2 is now the default XML parser specified using the XMLPARSER variable.
  * Set XMLPARSER=tinyxml or spirit to use the other parser options.

 * FREETYPE_CONFIG and XML2_CONFIG are used to specific the `freetype-config` and `xml2-config` programs used to configure libfreetype and libxml2.

 * GDAL (and the new OGR plugin) is now configured using the `gdal-config` program and not the GDAL_LIBS and GDAL_INCLUDES variables.
  * If `gdal-config` is not found use the GDAL_CONFIG variable to point to the full path of the program.

 * INPUT_PLUGINS now defaults to INPUT_PLUGINS=postgis,shape,raster so for other optional plugins you must add them to the list.
  * And easy shortcut is to specify INPUT_PLUGINS=all and then all plugins whose dependencies are found will be built. 

 * The install location for the Python bindings is now configured using `distutils` which may lead to a different install locations on Mac 10.5 and 64 linux systems.
  * What the configure or install output to see the location for where the python bindings will be installed.

# Building Mapnik with SCons

*GOCHAS*

 * Several optional dependencies (notably cairo, cairomm, and pycairo) are located using `pkg-config`. If you have more than one version you will need to modify the *PKG_CONFIG_PATH* variable to help pkg-config locate each dependency.

 * Make sure you have a C and C++ compiler installed before running SCons. Because SCons tests for the presence of Mapnik dependencies by building and running sample C code in the background, therefore you *must* first install your system's compilers (usually `cpp` and `g++`).

 * SCons accepts command-line `key=value` arguments to customize paths to look for dependecies as well as other valuable options.
  * All these arguments take the form of UPPERCASE = lowercase key value pairs. To see all possible options type:
        
```sh
    python scons/scons.py -h
```

  ..or if you have set custom configurations and want to see what affect they have had first `configure` then see the help:

```sh
    python scons/scons.py configure SOME_VARIABLE=some_value
    python scons/scons.py -h
```

 * If something goes wrong when finding dependencies make sure to look in the 'config.log' for any suspicious errors when SCons tried to compile the test targets. 

 * After a successful configure step, any  *custom* options (which differ from the defaults contained in the `SConstruct` files) will be saved in a user-editable 'config.py' file.
  * Specifying options in this 'config.py' file is an alternative to listing them on the commandline.
  * This file is just a python file so the `values` should be *quoted* if strings.

*Explanations of Commonly Used Options*

 * To get a view of the various customization options specific to building Mapnik run:

```sh
    python scons/scons.py -h
```

 * To see some overall SCons options run:

```sh
    python scons/scons.py -H
```

 * If during the `configure` stage Scons aborts because you are missing a dependency it will print the library name or the tool used to find libraries (ie xml2-config).

 * The first way to attempt to direct SCons to custom paths for your dependencies is to set the 'LIBS' and 'INCLUDES' variables for a given library, for example:

```sh
    python scons/scons.py configure BOOST_INCLUDES=/opt/local/includes/boost BOOST_LIBS=/opt/local/lib
```

 * If the tool used to find a library cannot be found then specify the custom path to the program:

```sh
    python scons/scons.py configure PG_CONFIG=/usr/lib/postgresql/8.3/bin/pg_config XML2_CONFIG=/usr/local/bin/xml2-config
```

 * Building Mapnik in debug mode can be a useful thing for new users or developers as it prompts printing of status output during rendering.
 * *NOTE*: Debug mode also means that Mapnik compilation is not optimized so Mapnik will run *slower*.

```sh
     python scons/scons.py configure DEBUG=True
```

  * Note: set `XML_DEBUG=True` in addition if you want to see boost spirit debug output during XML parsing.

 * As noted above SCons will inherit variables from a local 'config.py' files but if you don't want this to happen you can do:

```sh
    python scons/scons.py configure USE_CONFIG=False
```
  ...or you can point SCons at a custom list of locations for python files to inherit variables from:

```sh
    python scons/scons.py configure CONFIG=/home/dane/config.py,/opt/mapnik/custom_config.py
```

 * *NEW in version 0.6.0*
  * Libxml2 is now the default XMLPARSER which adds Entities support to Mapnik
  * But you can still configure Mapnik to build against tinyxml or spirit if you like:

```sh
    python scons/scons.py configure XML_PARSER=tinyxml
```

 * If you are experiencing any build problems it can be helpful to clean out all previous compiled files in the source directory:

```sh
    python scons/scons.py -c
```

 * Boost can often require extra variables (beyond the standard BOOST_INCLUDES and BOOST_LIBS) for SCons to properly find and link the boost library.
  * This is largely due to the different ways that boost installations create symlinks.
  * Often source installs of boost can be properly found by specifying the BOOST_TOOLKIT or BOOST_VERSION variables:

```sh
    python scons/scons.py configure BOOST_TOOLKIT=gcc43 BOOST_VERSION=1_37
```

 * The PostGIS, Shape, and Raster (TIFF) plugins are attempted to be build by default. To enable the compilation of further plugins either do:

```sh
    python scons/scons.py configure INPUT_PLUGINS=all
```

 or..

```sh
    python scons/scons.py configure INPUT_PLUGINS=postgis,shape,gdal,ogr,sqlite # etc...
```

 * Cairo is an optional dependency that will be built by default if the Cairo and Cairomm (C++ bindings to Cairo) can be found. To disable to default building of Cairo do:

```sh
    python scons/scons.py configure CAIRO=False
```

  * *Note*: if Cairo and Cairomm are found, pycairo will also be built by default to enable the python binding to Cairo (and therefore access to Cairo functions in Mapnik via the Mapnik python bindings).

 * SCons is very good at checking if the source code of any dependencies or the mapnik codes has changed, in order to know what targets to rebuild. This means that SCons builds can start slow.
 * To skip this behavior (only recommended for advanced users) at the cost of accurate builds turn on the FAST option with:

```sh
    python scons/scons.py configure FAST=True
```

 * If you have a machine with several processors you can run parallel builds to speed up compilation with the JOBS option:

```sh
    python scons/scons.py configure JOBS=2
```

 * Below is a dump of all the SCons options*
Note: this is based on a Mac OS 10.5 setup

```sh
        $ python scons/scons.py -h
        scons: Reading SConscript files ...
    
        Welcome to Mapnik...
    
        scons: done reading SConscript files.
    
        CXX: The C++ compiler to use (defaults to g++).
            default: g++
            actual: g++
    
        OPTIMIZATION: Set g++ optimization level (0|1|2|3)
            default: 2
            actual: 3
    
        DEBUG: Compile a debug version of Mapnik (yes|no)
            default: False
            actual: True
    
        XML_DEBUG: Compile a XML verbose debug version of mapnik (yes|no)
            default: False
            actual: False
    
        INPUT_PLUGINS: Input drivers to include
            (all|none|comma-separated list of names)
            allowed names: raster sqlite postgis ogr shape osm occi gdal
            default: raster,postgis,shape
            actual: gdal occi ogr osm postgis raster shape sqlite
    
        CONFIG: The path to the python file in which to save user configuration options. Currently : 'config.py'
            default: config.py
            actual: config.py
    
        USE_CONFIG: Use SCons user '%s' file (will also write variables after successful configuration) (yes|no)
            default: True
            actual: True
    
        FAST: Make scons faster at the cost of less precise dependency tracking (yes|no)
            default: False
            actual: False
    
        PREFIX: The install path "prefix"
            default: /usr/local
            actual: /usr/local
    
        PYTHON_PREFIX: Custom install path "prefix" for python bindings (default of no prefix)
            default: 
            actual: /Users/spring/mapnik-python/
    
        DESTDIR: The root directory to install into. Useful mainly for binary package building
            default: /
            actual: /
    
        BOOST_INCLUDES: Search path for boost include files ( /path/to/BOOST_INCLUDES )
            default: /usr/include
            actual: /usr/include
    
        BOOST_LIBS: Search path for boost library files ( /path/to/BOOST_LIBS )
            default: /usr/lib
            actual: /usr/lib
    
        BOOST_TOOLKIT: Specify boost toolkit, e.g., gcc41.
            default: 
            actual: 
    
        BOOST_ABI: Specify boost ABI, e.g., d.
            default: 
            actual: 
    
        BOOST_VERSION: Specify boost version, e.g., 1_35.
            default: 
            actual: 
    
        FREETYPE_CONFIG: The path to the freetype-config executable.
            default: freetype-config
            actual: freetype-config
    
        XML2_CONFIG: The path to the xml2-config executable.
            default: xml2-config
            actual: xml2-config
    
        ICU_INCLUDES: Search path for ICU include files ( /path/to/ICU_INCLUDES )
            default: /usr/include
            actual: /usr/include
    
        ICU_LIBS: Search path for ICU include files ( /path/to/ICU_LIBS )
            default: /usr/lib
            actual: /usr/lib
    
        PNG_INCLUDES: Search path for libpng include files ( /path/to/PNG_INCLUDES )
            default: /usr/include
            actual: /usr/include
    
        PNG_LIBS: Search path for libpng include files ( /path/to/PNG_LIBS )
            default: /usr/lib
            actual: /usr/lib
    
        JPEG_INCLUDES: Search path for libjpeg include files ( /path/to/JPEG_INCLUDES )
            default: /usr/include
            actual: /usr/include
    
        JPEG_LIBS: Search path for libjpeg library files ( /path/to/JPEG_LIBS )
            default: /usr/lib
            actual: /usr/lib
    
        TIFF_INCLUDES: Search path for libtiff include files ( /path/to/TIFF_INCLUDES )
            default: /usr/include
            actual: /usr/include
    
        TIFF_LIBS: Search path for libtiff library files ( /path/to/TIFF_LIBS )
            default: /usr/lib
            actual: /usr/lib
    
        PROJ_INCLUDES: Search path for PROJ.4 include files ( /path/to/PROJ_INCLUDES )
            default: /usr/local/include
            actual: /usr/local/include
    
        PROJ_LIBS: Search path for PROJ.4 library files ( /path/to/PROJ_LIBS )
            default: /usr/local/lib
            actual: /usr/local/lib
    
        INTERNAL_LIBAGG: Use provided libagg (yes|no)
            default: True
            actual: True
    
        CAIRO: Attempt to build with Cairo rendering support (yes|no)
            default: True
            actual: True
    
        GDAL_CONFIG: The path to the gdal-config executable for finding gdal and ogr details.
            default: gdal-config
            actual: gdal-config
    
        PG_CONFIG: The path to the pg_config executable.
            default: pg_config
            actual: pg_config
    
        OCCI_INCLUDES: Search path for OCCI include files ( /path/to/OCCI_INCLUDES )
            default: /usr/lib/oracle/10.2.0.3/client/include
            actual: /usr/lib/oracle/10.2.0.3/client/include
    
        OCCI_LIBS: Search path for OCCI library files ( /path/to/OCCI_LIBS )
            default: /usr/lib/oracle/10.2.0.3/client/lib
            actual: /usr/lib/oracle/10.2.0.3/client/lib
    
        SQLITE_INCLUDES: Search path for SQLITE include files ( /path/to/SQLITE_INCLUDES )
            default: /usr/include/
            actual: /usr/local/include
    
        SQLITE_LIBS: Search path for SQLITE library files ( /path/to/SQLITE_LIBS )
            default: /usr/lib
            actual: /usr/local/lib
    
        SYSTEM_FONTS: Provide location for python bindings to register fonts (if given aborts installation of bundled DejaVu fonts)
            default: 
            actual: 
    
        LIB_DIR_NAME: Name to use for lib folder where fonts and plugins are installed
            default: /mapnik/
            actual: /mapnik/
    
        PYTHON: Full path to Python executable used to build bindings ( /path/to/PYTHON )
            default: /System/Library/Frameworks/Python.framework/Versions/2.5/Resources/Python.app/Contents/MacOS/Python
            actual: /System/Library/Frameworks/Python.framework/Versions/2.5/Resources/Python.app/Contents/MacOS/Python
    
        FRAMEWORK_PYTHON: Link against Framework Python on Mac OSX (yes|no)
            default: True
            actual: True
    
        BINDINGS: Language bindings to build
            (all|none|comma-separated list of names)
            allowed names: python
            default: all
            actual: python
    
        THREADING: Set threading support (multi|single)
            default: multi
            actual: multi
    
        XMLPARSER: Set xml parser  (tinyxml|spirit|libxml2)
            default: libxml2
            actual: libxml2
    
        JOBS: Set the number of parallel compilations
            default: 1
            actual: 4
    
        DEMO: Compile demo c++ application (yes|no)
            default: False
            actual: False
    
        PGSQL2SQLITE: Compile and install a utility to convert postgres tables to sqlite (yes|no)
            default: False
            actual: True
    
        Use scons -H for help about command-line options.
```
