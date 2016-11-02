# Installing Mapnik on Mac OS X

## Binary Installers

Binary Mapnik installers for Mac OS X are available at [mapnik.org/pages/downloads.html](http://mapnik.org/pages/downloads.html)

## Using Homebrew

Homebrew is the recommended OS X package manager for installing Mapnik.

### Install Homebrew

First, make sure you have homebrew [installed](http://github.com/mxcl/homebrew/wiki/installation)

Then make sure homebrew is updated:

```sh
brew update
```

### Install Options

Now you can either:

 - Install Mapnik itself with homebrew (which will automatically install all dependencies at the same time)
 - Install all Mapnik dependencies via homebrew and then Mapnik from source.

#### Understanding build options

To see the options available for the build do:

```sh
brew info mapnik
```
For instance you should see `--with-cairo`, `--with-gdal`, and `--with-postgresql`. Unless you supply `--with-cairo` Mapnik will not be built with Cairo rendering support. If you pass `--with-gdal` or `--with-postgresql` then Homebrew will automatically install these dependencies before building Mapnik. 
In order to work with a PostGIS Database you have to supply `--with-postgresql` (the formula only adds PG/GDAL support if stated explicitly: [see formula](https://github.com/Homebrew/homebrew/blob/master/Library/Formula/mapnik.rb#L79-L80))

```sh
brew install mapnik
```

Note that the current `mapnik` package in Homebrew is based on Mapnik 3 and does not include the Python bindings by default. To install the older Mapnik 2.2 package with Python bindings:

```sh
brew install homebrew/versions/mapnik2
```

#### pycairo support

If you want both cairo rendering support and the ability to work with cairo objects (and pass them to mapnik) in python do:

```sh
brew install py2cairo
brew install mapnik --with-cairo
```

#### To install the latest Mapnik release from Homebrew do:

```sh
brew install cairo --without-x --without-glib
brew install icu4c
brew link icu4c
brew install boost
brew install boost-python
brew install proj
brew install jpeg
brew link jpeg
brew install libtiff
brew install gdal --with-libtiff=/usr/local/lib
brew install ossp-uuid
brew install postgis
brew install harfbuzz
git clone --recursive https://github.com/mapnik/mapnik.git
cd mapnik
./configure
make
make install
```

Note that on Lion, you need may to be more explicit about SQLite.  Change version as needed.

```
 ./configure CXX="clang++" JOBS=`sysctl -n hw.ncpu` SQLITE_LIBS=/usr/local/Cellar/sqlite/3.7.12/lib/ SQLITE_INCLUDES=/usr/local/Cellar/sqlite/3.7.12/include/
```

### Boost-Python Link Problems

After you install mapnik, you may try to import it and get `Fatal Python error: Interpreter not initialized (version mismatch?)`. If so, you likely have boost linked with the wrong version of python. To see what version of python boost is linked from, try:

```sh
otool -L `brew list boost | grep python-mt.dylib` | grep -i python
```

It's likely that your copy of boost was linked against the system python, but you're trying to use a homebrew python. To fix, uninstall boost, and reinstall with --build-from-source:

```sh
brew uninstall boost
brew install --build-from-source boost
```

### Mapbox Variant not found problem
After git clone of mapnik, execute the following to pull down additional dependencies
```
git submodule update --init deps/mapbox/variant
```

### Building with Cairo

If you need Cairo and its Python bindings, install and link these (cairo and py2cairo) with homebrew as normal. Then, to build Mapnik from source with Cairo:

```
./configure CXX="clang++" JOBS=`sysctl -n hw.ncpu` CAIRO=True PKG_CONFIG_PATH=/usr/lib/pkgconfig:/usr/local/lib/pkgconfig:/usr/X11/lib/pkgconfig
```

