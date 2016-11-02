<!-- Name: MacPythonUpgradeIssues -->
<!-- Version: 4 -->
<!-- Last-Modified: 2009/02/05 10:01:35 -->
<!-- Author: springmeyer -->
If you have upgraded to the MacPython binary off python.org and rebuilt your boost libs from source (to match your new python version) you may encounter the "Fatal Python error: Interpreter not initialized (version mismatch?)" error.

 * *Note*, as of version 1.35 Macports now automatically links its copy of boost its own copy of python. So, it is not possible to use boost built with Macports along with an upgraded python from MacPython/Python.org. If you wish to use Boost built from macports then use the `python-select` tool to switch your default version of python to the Macports version, then rebuild mapnik using that python. When installing a version of python with macports, it should output this hint:

```sh
    --->  Activating python25 @2.5.4_0+darwin_9+macosx
```

To fully complete your installation and make python 2.5 the default, please run
    
```
    sudo port install python_select  
    sudo python_select python25
```

The scenario goes as follows...

```sh
    $ sudo python scons/scons.py TIFF_INCLUDES=/usr/local/include
    JPEG_LIBS=/opt/local/lib JPEG_INCLUDES=/opt/local/include DEBUG=y
    BOOST_LIBS=/usr/local/lib BOOST_INCLUDES=/usr/local/include/boost-1_35
```

Mapnik will build just fine with your new python installation will not complain but when importing you get
the "Fatal Python error: Interpreter not initialized (version
mismatch?)". Check to see what your _mapnik.so is linking against.

```sh
    $ cd ~/src/mapnik
    springmeyer:mapnik spring$ otool -L bindings/python/_mapnik.so | grep python
    bindings/python/_mapnik.so:
    	/opt/local/lib/libboost_python-mt-1_35.dylib (compatibility version 0.0.0, current version 0.0.0)
```

If, like the above command result, you see that your mapnik module it linked against 'opt' (the macports dir), then either remove boost from macports or switch to the macports version of python. Even if you pointed the SCons path variables at a different version of boost, the SCons linker found the macports version because it was on the library path of another one of your dependencies (In the above example, the JPEG_LIBS).

The easiest solution is to uninstall boost from macports:

```sh
    $ sudo port uninstall boost
```

Also, make sure you do not have multiple installations of mapnik in several different site-packages locations. 

```sh
    locate _mapnik.so # should turn up only on install location
```

When you rebuild mapnik after uinstalling boost, mapnik will overwrite previous problematic libraries that were linked against the wrong version of boost, but only in the one place mapnik gets installed. So, hand remove any duplicated install locations.

Finally, follow the boost build instructions at MacInstallation and link up your boost libs. On leopard as of r738 the links in /usr/local/lib are:

```sh
    $ sudo ln -s libboost_system-mt-1_35.dylib libboost_system-mt.dylib
    $ sudo ln -s libboost_filesystem-mt-1_35.dylib libboost_filesystem-mt.dylib
    $ sudo ln -s libboost_regex-mt-1_35.dylib libboost_regex-mt.dylib
    $ sudo ln -s libboost_iostreams-mt-1_35.dylib libboost_iostreams-mt.dylib
    $ sudo ln -s libboost_program_options-mt-1_35.dylib
    libboost_program_options-mt.dylib
    $ sudo ln -s libboost_thread-mt-1_35.dylib libboost_thread-mt.dylib
    $ sudo ln -s libboost_python-mt-1_35.dylib libboost_python-mt.dylib
```

From here you should be able to rebuild, reinstall and run mapnik from the interpreter without a problem.