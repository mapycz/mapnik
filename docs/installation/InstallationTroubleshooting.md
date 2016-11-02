# Installation Troubleshooting

*See also: [[UsingScons]] for help on how to properly use SCons to find your dependencies.

## Build Errors

### Implicit dependency not found

 * *Description*: You get a scons build error like:

```
scons: *** [bindings/python/mapnik_color.os] Implicit dependency `/usr/include/float.h' not found, needed by target `bindings/python/mapnik_color.os'.
```

 * *Solution*: Either check out a new copy of mapnik or clear out the scons cache and re-configure:

```sh
make reset
./configure
make
```


### boost: object_base_initializer was not declared in this scope

 * *Description*: You get a compile error related to boost iostreams when building mapnik's shapefile or ogr plugin.

```sh
    g++ -o plugins/input/shape/shape.os -c -DHAVE_LIBXML2 -DHAVE_PYCAIRO - ansi -Wall -pthread -ftemplate-depth-100 -DLINUX - DBOOST_SPIRIT_THREADSAFE -DMAPNIK_THREADSAFE -DSVN_REVISION=1418 -O0 - fno-inline -g -DDEBUG -DMAPNIK_DEBUG -DSHAPE_MEMORY_MAPPED_FILE -fPIC - Iagg/include -Iinclude -I. -I/usr/include -I/usr/local/include -I/usr/ include/freetype2 -I/usr/include/libxml2 -I/usr/include/pycairo -I/usr/ include/cairo -I/usr/include/libpng12 plugins/input/shape/shape.cpp /usr/local/include/boost/iostreams/device/mapped_file.hpp:43: error: redefinition of  class boost::iostreams::mapped_file_base /usr/local/include/boost/iostreams/device/mapped_file.hpp:43: error: previous definition of âclass boost::iostreams::mapped_file_base
```

 * *Solution*: Upgrade boost as this is a bug in boost 1.41. Or patch a single header like:

```sh
    cat << EOF > iostream.patch
    --- boost/iostreams/device/mapped_file.hpp      2009-11-22 11:32:31.000000000 -0800
    +++ boost/iostreams/device/mapped_file.hpp      2009-11-22 11:24:25.000000000 -0800
    @@ -4,6 +4,9 @@
     // Distributed under the Boost Software License, Version 1.0. (See accompanying
     // file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt.)
    
    +#ifndef BOOST_IOSTREAMS_MAPPED_FILE_HPP_INCLUDED
    +#define BOOST_IOSTREAMS_MAPPED_FILE_HPP_INCLUDED
    +
     #if defined(_MSC_VER) && (_MSC_VER >= 1020)
     # pragma once
     #endif
    @@ -591,3 +594,5 @@
     } } // End namespaces iostreams, boost.
    
     #include <boost/config/abi_suffix.hpp> // pops abi_suffix.hpp pragmas
    +
    +#endif // #ifndef BOOST_IOSTREAMS_MAPPED_FILE_HPP_INCLUDED
    EOF
    patch -p0 < iostream.patch
```

### boost: object_base_initializer was not declared in this scope

 * *Description*: You get a compile error related to boost when building mapnik.

```sh
    /usr/local/include/boost/python/numeric.hpp:121: instantiated from ‘void boost::python::numeric::array::resize(const Sequence&) [with Sequence = boost::python::tuple]’ /usr/local/include/boost/preprocessor/iteration/detail/local.hpp: 37: instantiated from here /usr/local/include/boost/python/object_core.hpp:330: error: ‘object_base_initializer’ was not declared in this scope
    scons: *** [bindings/python/mapnik_shield_symbolizer.os] Error 1
    scons: building terminated because of errors.
```

 * *Solution*: You need to re-compile boost_python, correctly. Likely what is happening is that you have multiple python versions on your system, and when you built boost, the boost_python library compiled against a different set of headers than it linked against. Basically, `bjam`, the tool used to build boost, is not very smart about linking to python versions, and needs extra help. At the least you MUST pass the python version to `bjam` like:

```sh
    bjam --with-python python=2.6 [...snip...]
```

 * And sometimes even that does not work. *HINT:* pass the `-d2` flag to see all the compile commands sent to `gcc` by `bjam` and you will likely see something like `-I/usr/include/python24` in the compile arguments when it should be `-I/usr/include/python26` (or some older version of python headers). If this happens then you can craft a full config file (with all possible python info) and pass a reference to that on the bjam command line. Docs on this are here: http://www.boost.org/doc/libs/1_42_0/libs/python/doc/building.html#configuring-boost-build, and an example follows:

Create a file called 'user-config.jam' (but change the python versions to be appropriate):

    import option ;
    import feature ;
    if ! gcc in [ feature.values <toolset> ]
    {
        using gcc ;
    }
    project : default-build <toolset>gcc ;
    using python
         : 2.5 # version
         : /usr/bin/python2.5 # cmd-or-prefix
         : /usr/include/python2.5/ # includes
         : /usr/lib/python2.5/config/ # a lib actually symlink
         : <toolset>gcc # condition
         ;
    libraries = --with-python ;

Then, go recompile (just boost_python) and move the new library into place:

    ./bjam --with-python -a -j2 --ignore-site-config --user-config=user-config.jam toolset=gcc stage -d2
    #install them by hand as it is faster
    sudo cp stage/lib/libboost_python.so* /usr/local/lib/

### Dependency not found but once worked I know!
 * *Description*: One of the mapnik dependencies was once found by Scons, then cached, and even when you set the variable correctly (ie TIFF_INCLUDES=/correct/path) Scons does not find the library/include.

```sh
    Checking for C++ library X... no (cached)
    Could not find header or shared library for X, exiting!
```

 * *Solution*: Use the Scons option `--config=force` which will force Scons to forget about the old location used and honor your new command line variable/options.

### All boost libs are found except for boost_thread
 * *Description*: SCons apparently fails to find all boost libs except for boost_thread (or the last main C++ dependency checked) during the configure stage.

```sh
     Checking for C++ library boost_program_options-mt... yes
     Checking for C++ library boost_thread-mt... no
     Could not find required header or shared library for boost thread
```

 * *Solution*: Likely do to hitting ctrl-c or otherwise aborting the configure stage, SCons internal `.sconf_temp/` have been messed up. Delete them and rebuild:

```sh
    rm -fR .sconf_temp
```

### Boost/ICU linking error when linking libmapnik
 * *Description*: undefined symbols in boost_regex when linking Mapnik

```
    Undefined symbols:
      "boost::re_detail::icu_regex_traits_implementation::do_transform(int const*, int const*, icu::Collator const*) const", referenced from:
      unsigned short const* boost::re_detail::re_is_set_member<unsigned short const*, int, boost::icu_regex_traits, unsigned long long>(unsigned short const*, unsigned short const*, boost::re_detail::re_set_long<unsigned long long> const*, boost::re_detail::regex_data<int, boost::icu_regex_traits> const&, bool)in agg_renderer.os
      unsigned short const* boost::re_detail::re_is_set_member<unsigned short const*, int, boost::icu_regex_traits, unsigned long long>(unsigned short const*, unsigned short const*, boost::re_detail::re_set_long<unsigned long long> const*, boost::re_detail::regex_data<int, boost::icu_regex_traits> const&, bool)in agg_renderer.os
```

 * *Solution*: Likely, your boost library was built without regex+icu support. If you have built libboost from source, you can make your system's relevant .so files be recompiled with that support and reinstalled in place in the /lib/ folders. Go to libboost's source folder and run the following command (which you can find on [latest release install details](http://github.com/mapnik/mapnik/wiki/Mapnik2#fromlatestrelease|Mapnik2's)):

```sh
    sudo ./bjam --with-regex toolset=gcc -sHAVE_ICU=1 -sICU_PATH=/usr/local/ -a install
```

Note the -a setting which makes your system's installed regex symbols to be updated in place.

### Mapnik build error due to outdated ICU
 * *Description*: Mapnik build fails with errors mentioning "icu" and Unicode stuff.

```
    src/expression_string.cpp: In member function "void mapnik::expression_string::operator()(const mapnik::regex_match_node&) const":
    src/expression_string.cpp:79: error: "fromUTF32" is not a member of "icu_3_8::UnicodeString"
```

 * *Solution*: The above errors tells basically that function UnicodeString::fromUTF32(...) used in Mapnik code isn't provided by your current ICU installation (above is 3.8). You need at least 4.1.4 (see [#482](https://github.com/mapnik/mapnik/issues/482) for more details). Likely, you are compiling Mapnik from svn or a more recent version than the 7.0 branch with your distribution's (above was Ubuntu 8.04) outdated ICU package (version<4.1.4). Remove update your ICU package or both remove your distribution's ICU package (eg. sudo apt-get remove libicu*) and compile and install ICU from source (from [http://icu-project.org/download/](http://icu-project.org/download/)).

### Harfbuzz not found

    $ brew install harfbuzz
    ...
    $ ./configure
    ...
    Checking for C++ library harfbuzz... no
    Could not find required header or shared library for harfbuzz
    ...
    Exiting... the following required dependencies were not found:
     - harfbuzz (HarfBuzz text shaping library | configure with HB_LIBS & HB_INCLUDES)

 * *Solution*: HB_LIBS and HB_INCLUDES are directories, not cflags/ldflags, and HB_INCLUDES needs not to include the "harfbuzz" directory itself.

If pkg-config says:

    $ pkg-config --libs --cflags harfbuzz
    -I/usr/local/Cellar/harfbuzz/0.9.35_1/include/harfbuzz -L/usr/local/Cellar/harfbuzz/0.9.35_1/lib -lharfbuzz

what you actually need is

    $ ./configure HB_LIBS=/usr/local/Cellar/harfbuzz/0.9.35_1/lib HB_INCLUDES=/usr/local/Cellar/harfbuzz/0.9.35_1/include

### ICU not found

    $ python
    $ import mapnik
    from _mapnik import * 
    ImportError: libicudata.so.38: cannot open shared object file: No such file or directory

 * *Solution*: Mapnik's source code and libraries depend on Boost library (libboost*) which in turn -for the libboost regex library- depends on ICU library (libicu*). If you have compiled both libboost and libicu from source (eg. you wanted to build the bleeding edge Mapnik's svn/trunk) ; you likely did not remove your system's libicu* packages, build libboost first against those old ICU install libraries, then erased the latter libraries to install the ICU source newer libraries, which made libboost still linked to old libraries. In short, you should : a) build libUCI to update your system's installed version, b) build libboost that links to the new libUCIs.
 * *Solution*: This may simply occur because your mapnik library was compiled for a different ICU version than your currently installed ICU library. For example, your Mapnik was compiled and linked against libicu 3.8 while you have later on compiled and installed libicu v4.x. You should:

    1. check that libicu is installed and same version as the error (find -name "*libicu*.so*" /usr/ ).
    2. If the error's mentioned ICU version and the find installed ICU version do not differ, try running "$ ldconfig" as a superuser ; restart python and see if the python import command works now.
    3. If ICU versions did differ (error message <> installed version), you have install a newer/older Mapnik or ICU libraries so that Mapnik's ICU requirements and your install library match. Typically, if you compiled and installed Mapnik from SVN/trunk and libicu was installed with your package manager, remove the latter package first (eg. apt-get remove libicu* on Debian/Ubuntu) and compile+install libicu from source (take newest ICU4C or SVN : [http://site.icu-project.org/download](http://site.icu-project.org/download)).

 * *Description*: International Components for Unicode libs are installed but Scons says they could not be found.

```sh
    Checking for C++ library icuuc... no
    Could not find header or shared library for icuuc, exiting!
```

 * *Solution*: This is the first C++ program that Scons 'finds' by actually compiling a sample program that links to it. *A g++ compiler* is therefore _required_ for this step. On linux do `apt-get install g++` and on Mac os make sure you've installed XCode from the apple dev site.

 * *Solution*: If g++ is available then ICU must be in a custom location so set the paths to the libs and includes - ie. ICU_LIBS=/usr/local/lib and ICU_INCLUDES=/usr/local/include.

 * *Solution*: sudo aptitude install g++ make expat libicu-dev

### Boost not found during SCons build

 * *Description*: Often the various packages that install the Boost libraries do not properly symlink to the .so such that mapnik's Scons build process does not automatically find the boost libraries.

```sh
    Checking for C++ library boost_filesystem-mt... no
    Could not find header or shared library for boost_filesystem-mt, exiting!
```

 * *Solution Option 1*: Locate your libboost*.so files - usually located in /usr/lib, and find their naming scheme, then match it with the BOOST_TOOLKIT and BOOST_VERSION variables:

```sh
    $ ls /usr/lib/libboost_* -la
    lrwxrwxrwx 1 root root      40 2008-02-01 22:46 /usr/lib/libboost_filesystem-gcc-1_33_1.so -> libboost_filesystem-gcc-1_33_1.so.1.33.1
    -rw-r--r-- 1 root root   70152 2008-01-17 04:35 /usr/lib/libboost_filesystem-gcc-1_33_1.so.1.33.1
    lrwxrwxrwx 1 root root      43 2008-02-01 22:46 /usr/lib/libboost_filesystem-gcc-mt-1_33_1.so -> libboost_filesystem-gcc-mt-1_33_1.so.1.33.1
    -rw-r--r-- 1 root root   70248 2008-01-17 04:35 /usr/lib/libboost_filesystem-gcc-mt-1_33_1.so.1.33.1
    lrwxrwxrwx 1 root root      36 2008-02-01 22:46 /usr/lib/libboost_filesystem.so -> libboost_filesystem-gcc-mt-1_33_1.so
```

then, since 'threading' (mt) will be looked for by default you can match libboost*-gcc-mt-1_33_1.so with...

```sh
    $ python scons/scons.py configure BOOST_TOOLKIT=gcc BOOST_VERSION=1_33
```

 * *Solution Option 2*: Locate your libboost* files - usually located in /usr/lib and manually create symlinks like:
 
```sh
    cd /usr/lib
    ln -s libboost_filesystem.so libboost_filesystem-mt.so
    ln -s libboost_regex.so libboost_regex-mt.so
    ln -s libboost_iostreams.so libboost_iostreams-mt.so
    ln -s libboost_program_options.so libboost_program_options-mt.so
    ln -s libboost_thread.so libboost_thread-mt.so
    ln -s libboost_python.so libboost_python-mt.so
```

Note : if you experience trouble installing boost on *Ubuntu 7.04* see this mapnik-devel thread: https://lists.berlios.de/pipermail/mapnik-devel/2008-August/000700.html

Note: If the SCons build process simply can't find an include or library and you are about to dive into the SConscript code to muck around, first read: [SCons can't find an include header](http://www.scons.org/faq.php#SS_4_1)

### Boost undefined symbols

 * *Description*: Boost builds without problems but Mapnik will not run due to missing Boost symbols (functions)
*Related to filesystem symbols (with old libboost versions <1.35)*

```python
    Python 2.4.3 (#2, Aug 16 2008, 12:04:04)
    [GCC 4.1.1 20060724 (prerelease) (4.1.1-4pclos2007)] on linux2
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import mapnik
    Traceback (most recent call last):
      File "<stdin>", line 1, in ?
      File "/usr/lib/python2.4/site-packages/mapnik/__init__.py", line 31, in ?
        from _mapnik import *
    ImportError: /usr/local/lib/libmapnik.so.0.5: undefined symbol: _ZN5boost10filesystem6detail15not_found_errorE
```

 * *Solution*: This problem is old and likely fixed now whatever libboost source or recent package you'd take. According to [page](http://lists.boost.org/Archives/boost/2008/03/134329.php|this), the problem was fixed from libboost v1.35. _Rebuild boost and then mapnik. If this does not fix the problem, confirm that your Boost version is compatible with the Mapnik release version, see: BoostCompatibility

*Related to regex symbols with ICU support*

```python
    $ python
    >>> import mapnik
    Traceback (most recent call last):
       File "<stdin>", line 1, in <module>
       File "/usr/lib/python2.5/site-packages/mapnik/__init__.py", line 53, in <module>
         from _mapnik import * 
    ImportError: /usr/local/lib/libmapnik.so.0.8: undefined symbol: _ZN5boost11basic_regexIiNS_16icu_regex_traitsEE9do_assignEPKiS4_j
```

 * *Solution*: Likely, your boost library was built without regex+icu support. If you have built libboost from source, you can make your system's relevant .so files be recompiled with that support and reinstalled in place in the /lib/ folders. Go to libboost's source folder and run the following command (which you can find on [latest release install details](http://github.com/mapnik/mapnik/wiki/Mapnik2#fromlatestrelease|Mapnik2's)):

```sh
    sudo ./bjam --with-regex toolset=gcc -sHAVE_ICU=1 -sICU_PATH=/usr/local/ -a install
```

Note the -a setting which makes your system's installed regex symbols to be updated in place.
You may need to run "sudo ldconfig" after that command, then retry "import mapnik" in python to check.

### Input/Plugin not found error

 * *Description*: You rebuild Mapnik with Scons without a problem but when importing the python module you get a strange error about one of the input plugins

```python
    >>> import mapnik
    dlopen(/usr/local/lib/mapnik/input/postgis.input, 9): Library not loaded: /opt/local/lib/libboost_thread-mt-1_35.dylib
      Referenced from: /usr/local/lib/mapnik/input/postgis.input
      Reason: image not found
```
or

```python
    >>> import mapnik
    dlopen(/usr/local/lib/mapnik/input/gdal.input, 9): Library not loaded: /opt/local/lib/libboost_thread-mt-1_35.dylib
      Referenced from: /usr/local/lib/mapnik/input/gdal.input
      Reason: image not found
```

 * *Solution*: It is likely that Scons did not rebuild a given plugin because GDAL or PostGIS was not found. So, either rebuild with Scons again, ensuring that those optional libraries are found, or simply manually remove the *.input plugin file.

### Input/Plugin 'file not found' output
 * *Description*: You rebuild Mapnik with Scons without a problem but when importing the python module you strange output about 'file not found'.

```python
    >>> import mapnik
    file not found
    file not found
    file not found
    file not found
    file not found
```

 * *Solution*: A previous installation of Mapnik was compiled with more Plugins that your latest build. Either rebuilt with INPUT_PLUGINS=all or delete the unused plugin libraries which can be found by typing:

```python
    >>> import mapnik
    >>> print mapnik.inputpluginspath
```

## Cairo library not found

 * *Description*: Cairo plugin not found when rebuilding Mapnik with Cairo

 * *Solution*: Mapnik with Cairo needs the following libraries: libcairo2 libcairo2-dev python-cairo python-cairo-dev libcairomm-1.0-1 libcairomm-1.0-dev

## Linking Errors

### The libmapnik shared library is not found

 * *Description*: This is a common problem on linux encountered immediately after installing mapnik for the first time.

```python
    $ python
    >>> import mapnik
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
      File "/usr/lib/python2.5/site-packages/mapnik/__init__.py", line 31, in <module>
        from _mapnik import *
    ImportError: libmapnik.so.0.5: cannot open shared object file: No such file or directory
```

 * *Solution*: Add '/usr/local/lib' to the file 'etc/ld.so.conf' then run $ldconfig, log out and back in, and the shared library should be found. If you're running the 64bit version of Ubuntu, make the path '/usr/local/lib64'. Your Linux installation may have a directory '/etc/ld.so.conf.d' with additional configuration files - if this is the case and /'etc/ld.so.conf' contains "include /etc/ld.so.conf.d/*.conf", add a new file to 'etc/ld.so.conf.d' containing "/usr/local/lib".

### Mapnik not found after building on an AMD64 or x86 64bit machine

Do the same as above but you'll just need to add an entry of '/usr/local/lib64' into one of the files inside '/etc/ld.so.conf.d/'

## Install Errors

### Permission Denied Error

 * *Description*: Installing mapnik libs requires root priveleges

```python
    scons: Building targets ...
    Install file: "src/libmapnik.so" as "/usr/local/lib/libmapnik.so.0.5.0"
    scons: *** [/usr/local/lib/libmapnik.so.0.5.0] /usr/local/lib/libmapnik.so.0.5.0: Permission denied
    scons: building terminated because of errors.
```

 * *Solution*: Run:

```
    sudo python scons/scons.py install
```

## Python Errors

### Python TypeError

 * *Description*: In rare circumstances on Mac OS X the Mapnik python bindings are directly linked against a different version of Python then they were compiled against and you will see an error like:

```python
    TypeError: __init__() should return None, not 'NoneType'
```

 * *Solution*: In general you must rebuild Mapnik to link against the same version of Python as you are running. On Mac OS X recent versions of Mapnik >= 0.7 should prevent this from happening, but just in case you can manually use the `FRAMEWORK_SEARCH_PATH` option to point SCons (see [[UsingScons]]) at the directory where the desired 'Python.framework' exists.

### Python Version Mismatch

 * *Description*: A `Python Mismatch` can occur when the version of boost_python you installed is linked to a different version of Python that you are running and that Mapnik has linked to. This is most frequently seen when building boost with Macports, since Macports installs its own duplicate version of Python. See: http://trac.macports.org/ticket/17975

 * Since the Mapnik python bindings use boost python, both boost python (when it is compiled), and mapnik python must be linked against the same version of Python.

```python
    $ python
    Python 2.5 (r25:51918, Sep 19 2006, 08:49:13) 
    [GCC 4.0.1 (Apple Computer, Inc. build 5341)] on darwin
    Type "help", "copyright", "credits" or "license" for more information.
    >>> import mapnik
    Fatal Python error: Interpreter not initialized (version mismatch?)
    Abort trap
```

 * *Solution*: Usually you must rebuild boost_python so that it links against the correct version of Python, or you can use the `install_name_tool` to on-the-fly change the linking paths. In rarer cases you may need to rebuild Mapnik as well.

 * The `otool` command is how you find out what is linked against what library. In the following example, Boost python and Mapnik are both linked against the Python25 Apple installed version of Python on 10.5 (which works):

```
    $ otool -L /opt/local/lib/libboost_python-mt.dylib | grep Python
    	/System/Library/Frameworks/Python.framework/Versions/2.5/Python (compatibility version 2.5.0, current version 2.5.1)
    $ otool -L /Library/Python/2.5/site-packages/mapnik/_mapnik.so | grep Python
    /Library/Python/2.5/site-packages/mapnik/_mapnik.so:
    	/System/Library/Frameworks/Python.framework/Versions/2.5/Python (compatibility version 2.5.0, current version 2.5.1)
```

 * In Mapnik versions >=0.7, the FRAMEWORK_SEARCH_PATH option can be used to direct mapnik to link against a specific version of python, for example to link against Python 26 installed from python.org (which is /Library/Frameworks/Python.framework) do:

```
    $ python scons/scons.py FRAMEWORK_SEARCH_PATH=/Library/Frameworks
```

 * Also see this discussion: [MacPythonUpgradeIssues](https://github.com/mapnik/mapnik/wiki/MacPythonUpgradeIssues)

### unknown required load command

 * *Description*: You installed mapnik on osx using a binary installer. When you import the Mapnik python bindings you get a generic 'no suitable image found'.

```python
    >>> import mapnik
    Traceback (most recent call last):
      File "<stdin>", line 1, in <module>
      File "/Library/Frameworks/Mapnik.framework/Versions/Current/Python/System/2.5/site-packages/mapnik/__init__.py", line 53, in <module>
        from _mapnik import *
    ImportError: dlopen(/Library/Frameworks/Mapnik.framework/Versions/Current/Python/System/2.5/site-packages/mapnik/_mapnik.so, 258): Library not loaded: /Library/Frameworks/Mapnik.framework/Versions/0.7/Mapnik
      Referenced from: /Library/Frameworks/Mapnik.framework/Versions/0.7/Python/System/2.5/site-packages/mapnik/_mapnik.so
      Reason: no suitable image found.  Did find:
    	/Library/Frameworks/Mapnik.framework/Versions/0.7/Mapnik: unknown required load command 0x80000022
    	/Library/Frameworks/Mapnik.framework/Versions/0.7/Mapnik: unknown required load command 0x80000022
    	/usr/local/lib/Mapnik: not a file
```

 * *Solution*: The key error is "unknown required load command" which indicates that you are likely trying to run libraries compiled for a different MAC OS X version. Go back and make sure you installed the correct Installer for your OS X version.


## Runtime Errors

### Boost assertion error in visitation: "Assertion false failed"

 * *Description*: When trying to run a program written in C/C++ that has been compiled against mapnik, boost throws an assertion error that looks like:

```
    /usr/include/boost/variant/detail/visitation_impl.hpp:203: typename Visitor::result_type boost::detail::variant::visitation_impl(int, int, Visitor&, VPCV, mpl_::true_, NBF, W*, S*) [with W = mpl_::int_<20>, S = boost::detail::variant::visitation_impl_step<boost::mpl::l_iter<boost::mpl::l_end>, boost::mpl::l_iter<boost::mpl::l_end> >, Visitor = boost::detail::variant::invoke_visitor<mapnik::symbolizer_attributes>, VPCV = const void*, NBF = boost::variant<mapnik::point_symbolizer, mapnik::line_symbolizer, mapnik::line_pattern_symbolizer, mapnik::polygon_symbolizer, mapnik::polygon_pattern_symbolizer, mapnik::raster_symbolizer, mapnik::shield_symbolizer, mapnik::text_symbolizer, mapnik::building_symbolizer, mapnik::markers_symbolizer, mapnik::glyph_symbolizer>::has_fallback_type_, typename Visitor::result_type = void, mpl_::true_ = mpl_::bool_<true>]: Assertion `false' failed.
```

 * *Solution*: This indicates a problem in compiling against mapnik headers. It might be able to be fixed simply by completely cleaning and rebuilding the C++ program that is linking to mapnik. If that does not solve it then this indicates that key compiler flags that affect how inlining are done were likely missing from the compile commands for the program that is building against mapnik headers. You should be using the `mapnik-config --cflags` output when compiling c++ programs against mapnik to ensure that key flags like these are included (but these may change between mapnik releases to check `mapnik-config`):

```
    -ansi -Wall -ftemplate-depth-200 -DBOOST_SPIRIT_THREADSAFE -DMAPNIK_THREADSAFE -O3 -finline-functions -Wno-inline -DNDEBUG
```

### Proj_init_error

 * *Description*: When trying to project a Map or Layer by setting a projection using the '+init=epsg:' syntax, mapnik returns a blank graphic. If mapnik was built in debug mode (scons.py DEBUG=y) then in your terminal (or apache logs when running the OGCServer) you will see a 'proj_init_error' during the map processing stage:

```sh
    start map processing bbox=Envelope(-180,-4.428969359331489,-94.42896935933148,81.14206128133704)
    proj_init_error:failed to initialize projection with:+init=epsg:4326
    end map processing
```

 * *Solution*: Reinstall your proj4 library because this means that mapnik (through proj4) can't find the proj epsg file. It could also mean that you are improperly setting the 'allowedepsgcodes' in the OGCServer conf file (which need a trailing ',' to be a formal python tuple) or that you are simply trying to initiate a custom projection that does not have a corresponding epsg code (in this case you'll need to add a record to your proj epsg file).
 * *Note*: If you want to use the Google Spherical Mercator projection and initiate the projection with an epsg code add this text to your /usr/local/share/proj/epsg file:

```sh
    <900913> +proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +no_defs +over<> 
```

### Node 'Map' not found

 * *Description*: When trying to use XML entities within an xml mapfile the load_map() function fails on a strange error:

```
    Not a map file. Node 'Map' not found.
```

 * *Solution*: Rebuild mapnik with support for XML entities by installing libxml2 and specifying the XMLParser as a Scons option. Libxml2 is not the default parser used by Mapnik but the only one capable of handling XML entities (see: [[MapnikXMLsupport]])

```sh
    python scons/scons.py install XMLPARSER=libxml2
```

### Expected type double but got ...

 * *Description*: When loading an xml map, mapnik throws a parsing error like:

```
    Error: Failed to parse attribute 'stroke-width'. Expected type double but got '.2' in LineSymbolizer in style 'style'
```

 * *Solution*: Notice in the above error that '.2' is a double type. This signals that C++ thinks doubles are not doubles, which is very bad. This is certainly an ABI compatibility problem and due to some problem compiling or linking. One potential cause is that libmapnik is linked to a different version of libstd++ or libgcc than libboost was or python or some other key mapnik dependency. So, most likely you have some bad duplicate installation of gcc/g++. Look around your system and try to clean it up otherwise no amount of recompiling mapnik will help. Once you find the culprit remove it and recompile mapnik. To find what libraries and linked against what other libraries use the `ldd` tool on linux and the `otool -L` command on os x.