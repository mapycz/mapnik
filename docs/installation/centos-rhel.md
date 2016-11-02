# CentOS/RHEL Installation

*Note:* These instructions are out of date. Until these are updated better references are:

* http://svn.osgeo.org/osgeo/foss4g/benchmarking/wms/2010/mapnik/scripts/install_mapnik_rhel_5.5.sh
* http://krisarnold.com/2010/07/14/installing-mapnik-on-centos-5/

## Tips for installing Mapnik-2.1.0 on RHEL 5.5

* Make sure your boost version is higher than 1.47. A completely reinstallation of boost is preferred if there is already an older version on your machine.
* Make sure your gcc version is higher than 4.1. The default gcc delivered with RHEL 5.5 does not meet this requirement. An internal compiler error will occur near make_rtl_for_nonlocal_decl in boost during the compilation. Upgrading to gcc44 (included in yum repository with RHEL 5.5) can solve the problem.

## Installation Notes
See also: https://lists.berlios.de/pipermail/mapnik-users/2010-March/003034.html

Some tips and hints for installing Mapnik on CentOS 5.2, default "Server" install.

All this is done as root.  You don't HAVE to be root, but doing it as a regular user is much more complicated and I'm not going to document it here.

You will need to add, at the very LEAST, the following development packages:


    # yum install freetype-devel
    # yum install libtool-ltdl-devel
    # yum install libpng-devel
    # yum install libtiff-devel
    # yum install libjpeg-devel
    # yum install gcc-c++
    # yum install libicu-devel
    # yum install python-devel
    # yum install bzip2-devel

This will install all necessary dependencies also!

Although CentOS comes with a version of Boost, it is not complete, and it's unclear as to whether it was compiled with multi-threaded support.

Therefore, I decided to download and compile my own (1.37 as of this writing).

You can get the latest version [here](http://www.boost.org/users/download/).

Installing it is fairly trivial, using the usual paradigm, though compilation can take a while.

I did:


    # ./configure --with-libraries=all
    # make
    # make install

Then, obtain compile and install [PROJ.4](http://trac.osgeo.org/proj/).  Again the usual paradigm of "configure", "make", "make install" works great.

You will need to make sure /usr/local/lib is in the path to the linker so all these now libraries will be found.  You do this by adding a file in:

/etc/ld.so.conf.d

that contains:

/usr/local/lib

and then run "ldconfig".  Do this now, after installing Boost and PROJ.4.

You can now download and install Mapnik.

Once you've extracted the source tree from the archive you downloaded, run:


    # python scons/scons.py BOOST_INCLUDES=/usr/local/include/boost-1_37/ BOOST_TOOLKIT=gcc41 install

This will build and install the Mapnik library, the default fonts, the input plugins and the Python bindings.

Run "ldconfig" once more to make sure the linker sees the new Mapnik stuff.

And finally, run the python demo, just to make sure everything works OK.  The demo is in the source tree, under demo/python.


    # cd /path/to/source/demo/python
    # python rundemo.py
    
    
    Three maps have been rendered in the current directory:
    - demo.jpg
    - demo.png
    - demo256.png
    
    Have a look!
    
    
    # 

If you see that output, no errors, and find those 3 images in the current directory, all went well.

Otherwise, time to debug, or ask the mailing list for help!

## Red Hat 4.6

I did need to install extra packages to install on Red Hat 4.6:

    # yum install libxml2-devel
