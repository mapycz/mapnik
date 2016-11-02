<!-- Name: MacInstallationSource -->
<!-- Version: 14 -->
<!-- Last-Modified: 2010/01/29 15:03:43 -->
<!-- Author: springmeyer -->

## WARNING - this page is out of date: Use Brew instead
See [MacInstallation_Brew](https://github.com/mapnik/mapnik/wiki/MacInstallation_Homebrew) for instructions.

# Installing Mapnik on Mac OS X (build dependencies from source)

If you are new to compiling source code, or having trouble getting started on a Mac, you may find that building Mapnik on a Linux-based system is a more friendly place to start. Also see MacInstallation which makes use of MacPorts instead.

## Notes on These Instructions

 * Prerequisites are mandatory unless noted

 * Instructions here focus on terminal commands. If you are new to command line usage, read up on using the [Bash shell](https://help.ubuntu.com/community/UsingTheTerminal?action=show&redirect=BasicCommands) and the mac native [Terminal.app](http://www.oreilly.com/pub/a/mac/2004/02/24/bash.html).

 * The [Wget](http://www.gnu.org/software/wget/) command is used below to download source packages and is not pre-installed on Mac OS.

 * If you are new to Python a great place to start is to ['Dive into Python'](http://www.diveintopython.net/installing_python/shell.html).

 * The $ indicates a normal command prompt in the shell.
 * The # indicates a command that likely needs to be run by a superuser.
 * The ## indicates code comments that are not executed.
 * If you run into any problems not discussed on this page see: InstallationTroubleshooting

## Prerequisites

 * Intel or PPC Mac, g++ compiler (default compiler should be fine)

 * *Python* ≥2.4 - [python.org](http://www.python.org)
  * Note: Python 2.5.1 comes preloaded on 10.5
  * Current Source (2.5) and Mac Binaries are available - http://www.python.org/download/

 * *The Latest Mapnik Source* - [0.5.1 ](http://mapnik.org/download/Release)
  * Download stable release 0.5.1 - [Mapnik Source 0.5.1](http://prdownload.berlios.de/mapnik/mapnik_src-0.5.1.tar.gz)
  * Checkout the current repository with subversion:

```sh
    git clone git://github.com/mapnik/mapnik.git mapnik
```
 
 * *Recommended*: Proj.4 - [Cartographic Projections Library](http://www.remotesensing.org/proj/)
  * necessary for reprojection support
  * necessary for the WMS server

 * *Boost* - [C++ Libraries](http://www.boost.org/)
  * Thread - libboost_thread-mt.dylib
  * System -  config.hpp (for different operating systems support)
  * Filesystem - libboost_filesystem.dylib
  * regex (regular expressions) - libboost_regex.dylib
  * Iostreams (input/output) - libboost_iostreams.dylib
  * Program Options - program_options.hpp
  * python - libboost_python.dylib
  * Note: [Boost Jam](http://www.boost.org/users/download/) (bjam) needed for boost source build

 * *Libraries* for Imaging, character encoding, font, compression, and parsing
  * libpng - [Portable Network Graphics format](http://www.libpng.org/pub/png/libpng.html)
  * libjpeg - [Joint Photographic Experts Group format](http://www.ijg.org/)
  * libtiff - [Tag Image File Format](http://www.libtiff.org/)
  * libltdl - [GNU Libtool / Shared Library Linker](http://www.gnu.org/software/libtool/manual/libtool.html) 
  * libz - [zlib compression library](http://www.zlib.net/)
   * Comes pre-installed on Mac OS X in /usr/local/lib
  * libicu -  [International Components for Unicode](http://www.icu-project.org/)
   * Note: libicucore.dylib is installed on mac but to build against it you need to generate the headers, see below
  * Optional: libcairo, libcairomm, pycairo, tinyxml / spirit / libxml2. See [MacInstallation/Optional] for more details.

## Instructions

### Short Version

1. Install dependencies (Good luck! See below...)

2. Optional: within the Mapnik source directory run this command to get your build options:
 * These are configured in the [source:trunk/SConstruct SConstruct file used by scons.py] but can be assigned with flags

```sh
    $ python scons/scons.py -h # lowercase h is the scons dependency configurations
    $ python scons/scons.py -H # capitol H is the scons help
```

3. Then run scons to build:

 * Note: this will only work if all your required dependencies are in the default locations (near impossible on Mac OS)
 * Some packages are located in scons using pkg-config (so make sure that binary in is your path)
 * Set custom directives with flags like PGSQL_INCLUDES=/usr/local/pgsql/include PGSQL_LIBS=/usr/local/pgsql/lib to point to custom locations

```sh
    $ python scons/scons.py
```

4. Once scons can find all needed dependencies it will compile Mapnik locally. To install then issue:

```sh
    $ python scons/scons.py install [and any of the flags you needed to find dependencies]
```

5. Optional: Run this command to clear out the local scons build if you already attempted, unsuccessfully, to build:

```sh
    $ python scons/scons.py -c
```

6. Optional: if you are rebuilding Mapnik due to compile problems or updgrading it may help to first delete your mapnik libs and python bindings:

```sh
    ## If you ran scons using the system python the PYTHON_PREFIX will likely be equivalent to: /usr/lib/python2.5/site-packages/
    $ export PYTHON_PREFIX=/System/Library/Frameworks/Python.framework/Versions/2.5/lib/python2.5/site-packages
    ## The PREFIX will be /usr/local by default or your custom location
    $ export PREFIX=/usr/local
    ## Uninstall the Python `mapnik` module.
    $ sudo rm -fr $PYTHON_PREFIX/mapnik
    ## Uninstall the mapnik libs, includes, and bin
    $ sudo rm -fr $PREFIX/include/mapnik
    $ sudo rm -fr $PREFIX/lib/mapnik
    $ sudo rm -f $PREFIX/lib/libmapnik.*
    $ sudo rm $PREFIX/bin/shapeindex
```

## Full Instructions

### Download Mapnik
1. Download the Mapnik source code from svn

```sh
    $ git clone git://github.com/mapnik/mapnik.git mapnik_svn
    $ cd mapnik_svn
```


2. Read over the INSTALL instructions for your version

```sh
    open INSTALL # Doc in the root of Mapnik download
```

### Optional Installs

3. Install Proj.4, PostgreSQL/PostGIS, and GDAL if necessary.

See [[MacInstallation_Optional]] for details.

### Install Necessary Libraries

4. Install all the imaging, character encoding, font, compression, and parsing libraries

 * This can be done with [Macports](http://www.macports.org/), [Fink](http://www.finkproject.org/), or by source (recommended).
 * The image libraries should already be installed if you used the [Frameworks](http://www.kyngchaos.com/wiki/software:frameworks) from kyngchaos.com for GDAL
  * You will still need to specify their locations in the scons build process like:
   * TIFF_LIBS=/Library/Frameworks/UnixImageIO.framework/Versions/Current/unix/lib
   * TIFF_INCLUDES=/Library/Frameworks/UnixImageIO.framework/Versions/Current/unix/include
  * And you may need to specify in your .bash_profile: 
   * export DYLD_LIBRARY_PATH=/Library/Frameworks/UnixImageIO.framework/Versions/Current/unix/lib
 * Detailed descriptions - TO DO / FIXME

### Install ICU

OS 10.5.10.6 trick to build against Apple provided ICU, which lacks headers.

Note: custom approach - it is easier to just install your own copy of ICU.

To get this to work you need to:

 * Pass `ICU_LIB_NAME=icucore` when building Mapnik for this to work (only 0.7 or greater).
 * compile boost_regex with:
 
```
    bjam --with-regex toolset=darwin -sHAVE_ICU=1 cxxflags='-DU_HIDE_DRAFT_API -DUDISABLE_RENAMING' -sICU_PATH=/usr/ -a install  
```

 * patch boost regex jamfile like:

```diff
    Index: libs/regex/build/Jamfile.v2
    ===================================================================
    --- libs/regex/build/Jamfile.v2	(revision 58875)
    +++ libs/regex/build/Jamfile.v2	(working copy)
    @@ -25,9 +25,9 @@
           if $(HAVE_ICU) && ! $(ICU_PATH)
           {
              gHAS_ICU = true ;
    -         gICU_CORE_LIB = icuuc ;
    -         gICU_IN_LIB = icui18n ;
    -         gICU_DATA_LIB = icudata ;
    +         gICU_CORE_LIB = icucore ;
    +         #gICU_IN_LIB = icui18n ;
    +         #gICU_DATA_LIB = icudata ;
              gICU_CONFIG_CHECKED = true ;
           }
           else if $(ICU_PATH)
```

And compile icu to pull out headers like:

```
    cd ~/src
    ### SNOW LEOPARD - 10.6
    curl -O http://www.opensource.apple.com/tarballs/ICU/ICU-400.38.tar.gz
    tar xvf ICU-400.38.tar.gz
    cd ICU-400.38
    
    ### LEOPARD - 10.5
    curl -O http://www.opensource.apple.com/tarballs/ICU/ICU-8.11.4.tar.gz
    rac xvf ICU-8.11.4.tar.gz
    cd ICU-8.11.4
    
    ## BUILD
    make
    sudo make install
    # copy headers to /usr/include
    # or you could put these anywhere that Mapnik can find them
    sudo mv build/usr/local/include/unicode/ /usr/include/
```

### Install Boost

5. Install Boost using Bjam

 * You can install boost with Macports (currently 1.35.0):

```sh
    $ port info boost-jam boost # optional: this is to check that these exist and see their details
    # port install boost-jam # must install bjam manually first, due to [http://trac.macosforge.org/projects/macports/ticket/13714 Macports bug]
    # port install -v boost +python25 +icu
```

  * Prepare for a long wait and lots of CPU!
  * Macports should automatically download any dependencies needed for Boost

 * Or install from source (recommended) like so:
  * Thanks jbronn for this writeup
  * Use the `bjam` not the `configure` script to build.
  * If you have a dual-core system you can add the `-j2` flag to both `bjam` and `scons` to take advantage of both cores:

```sh
    svn co http://svn.boost.org/svn/boost/trunk boost-trunk
    cd boost-trunk
    export BOOST=`pwd`
    cd tools/jam/src
    ./build.sh darwin
    cd bin.mac*/
    export PATH=`pwd`:$PATH
    cd $BOOST
    bjam --toolset=darwin \
      --with-thread --with-filesystem --with-iostreams \
      --with-regex --with-program_options --with-python \
      --with-system stage
    sudo bjam --toolset=darwin \
      --with-thread --with-filesystem --with-iostreams \
      --with-regex --with-program_options --with-python \
      --with-system install
```

### Install WMS dependencies

6. See [[MacInstallation_Optional]] for details.

### Finally, build and install Mapnik

7. Run scons (debug mode flag, DEBUG=y, is recommended for troubleshooting later on)

```sh
    $ cd mapnik-0.5.1
    $ python scons/scons.py DEBUG=y
```

 * If the scons build process is not able to find a required dependency it will exit.
  * You'll need to manually set the paths to the locations of library (libs) and/or header files (includes)
  * See the svn INSTALL readme for more details about setting paths or issue the command:

 * You will be looking for an output once scons finds all necessary dependencies (an elusive target) that looks like:

```sh
    $ python scons/scons.py DEBUG=y
    scons: Reading SConscript files ...
    Building on Darwin ...
    pkg-config --exists cairomm-1.0
    Checking for C library m...  yes
    Checking for C library ltdl...  yes
    Checking for C library png...  yes
    Checking for C library tiff...  yes
    Checking for C library z...  yes
    Checking for C library jpeg...  yes
    Checking for C library proj...  no
    Checking for C library pq...  no
    Checking for C++ library icuuc...  yes
    Checking for C++ library icudata...  yes
    Checking for C++ library gdal...  no
    Checking for C++ library boost_filesystem-mt...  yes
    Checking for C++ library boost_regex-mt...  yes
    Checking for C++ library boost_iostreams-mt...  yes
    Checking for C++ library boost_program_options-mt...  yes
    Checking for C++ library boost_thread-mt...  yes
    Bindings Python version... 2.5
    Python 2.5 prefix... /System/Library/Frameworks/Python.framework/Versions/2.5
    pkg-config --exists pycairo
    scons: done reading SConscript files.
    [... build process will commence...]
```

 * Mapnik should be successfully built at the culmination of that command. 

 * If this fails with an error complaining that boost filesystem cannot be found, check the section above for building boost and add the symlinks. Also ensure you add the following to the scons command (assuming built from source and default installation folder)

```sh
    $ python scons/scons.py DEBUG=y BOOST_LIBS=/usr/local/lib BOOST_INCLUDES=/usr/local/includes/boost-1_35/
```

 * Then you should install the compiled files:

```sh
    $ python scons/scons.py install DEBUG=y
```

### Test Installation

8. Now test your mapnik installation in the python interpreter:

```python
    $ python
    >>> import mapnik
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
    ImportError: No module named mapnik
```

 * Whoops, the mapnik module is not on sys.path! 

 * This command will show you what *is* on your PYTHONPATH:

```python
    $ python
    >>> import sys
    >>> sys.path
    ['', '/Library/Python/2.5/site-packages/ ....
```

9. If you got the ImportError put your mapnik module in your PYTHONPATH like so:
 * Place this text in either /etc/profile or ~/.bash_profile

```sh
    export PYTHONPATH=/usr/lib/python2.5/site-packages/
```

 * Or Symlink to your Mapnik module from the default site-packages directory (it should be on your path automatically)

```sh
    # ln -s /System/Library/Frameworks/Python.framework/Versions/2.5/lib/python2.5/site-packages/mapnik /Library/Python/2.5/site-packages/mapnik
```

 * Find your default site-packages directory like so:

```sh
    $ python -c "from distutils.sysconfig import get_python_lib; print get_python_lib()"
```

10. Now return to the python interpreter:

```sh
    $ python
    >>> import mapnik
    registered datasource : gdal
    registered datasource : postgis
    registered datasource : raster
    registered datasource : shape
    >>> dir(mapnik) # This gets you a list of symbols
    ['BoostPythonMetaclass', 'Color', 'Coord', 'CreateDatasource'
     [.....]
     'scale_denominator', 'setdlopenflags']
    >>> help(mapnik)
```

 * Congrats: you can now head to the [GettingStarted Getting Started page] for a 'Hello World' demo of using Mapnik via the python bindings.

## Known Issues

 * There are several known issues with building Mapnik on Mac OS X

 * It is entirely possible to run Mapnik on Mac 10.4 and 10.5 but some users have experienced the following problems:
  * Mapnik does not correctly read the map Envelope(extent) and gets caught in infinite loop during shapefile processing - [April 17](https://lists.berlios.de/pipermail/mapnik-users/2008-April/000801.html) - May be related to PPC architecture. See Ticket #92
  * NoneType Error occurs in python bindings - [April 21](https://lists.berlios.de/pipermail/mapnik-devel/2008-April/000610.html) & [April 18](https://lists.berlios.de/pipermail/mapnik-users/2008-April/000813.html). The NoneType error may be due to a previously installed version of Python (possible from Tiger to Leopard Upgrade) that left frameworks in /Library/Frameworks/Python.framework. Remove that folder, and rebuild mapnik. NOTE: Only do this if you can confirm that this is not the current location of your system Python.

 * Keep in mind that installation on most linux distros is an easier starting point. For example, on [UbuntuInstallation Ubuntu] many of the dependencies are either pre-installed or readily available via the synaptic package manager. Macports and Fink hypothetically offer similar convience but conflicts between local source installs and port/fink installs are pita's for mapnik mac users.

 * If you upgraded your core python (from the mac binary from python.org) and are trying to rebuild mapnik see this page [MacPython upgrade issues](MacPythonUpgradeIssues)