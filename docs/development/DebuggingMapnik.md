<!-- Name: DebuggingMapnik -->
<!-- Version: 4 -->
<!-- Last-Modified: 2009/10/05 12:49:44 -->
<!-- Author: phispi -->
This page exists to discuss the best way for developers to debug Mapnik.

Currently I have these questions:

 * What's the best way to get a Python build with debugging symbols? Will adding -g suffice? Is --with-pydebug needed?
 * Do we need to compile the Boost libraries with debugging symbols as well?
 * Is anything different for FreeBSD?


## Eclipse
Here I try to describe my setup. I'm sure there are better ways but this works for me:

* Checkout the latest code: `git clone git://github.com/mapnik/mapnik.git mapnik_trunk`

This has the advantages that

* you always have the newest version (`git pull`)
* you can quickly add test code and remove it again (`svn checkout .`)
* you can easily create patches (`git diff`)
* Compile it as described in the INSTALL file provided in the root directory of the checkout. I have the following variables in my `config.py`:

```python
    DEBUG = True
    XML_DEBUG = True
    INPUT_PLUGINS = 'gdal,ogr,osm,postgis,raster,shape,sqlite'
    PREFIX = '/home/philipp/usr'
    PYTHON_PREFIX = '/home/philipp/usr'
    BOOST_INCLUDES = '/usr/include'
    BOOST_LIBS = '/usr/lib'
    BINDINGS = 'all'
    DEMO = True
```

* Download and install the [Eclipse IDE for C/C++ Developers (79 MB)](http://www.eclipse.org/downloads/)
* Create a new Makefile-C++-Project using the existing `mapnik_trunk` path where you have checked out the working copy.
* Create a debug configuration e.g. specifying the c++ demo application `rundemo` in `demo/c++`.
* You can got through the program now step by step and make breakpoints. In the default behavior, Eclipse tries (and fails) to compile the project itself and ask to start anyway. Just say yes.
* If you change a file, recompile the project from the command line by just typing `python scons/scons.py`.
* It is possible (but not necessary) to fine-tune Eclipse so that it does not try to build the project by itself and that is uses scons to do successfully do so.

## References

* http://wiki.python.org/moin/DebuggingWithGdb
* http://www.boost.org/libs/python/doc/v2/faq.html#debugging 