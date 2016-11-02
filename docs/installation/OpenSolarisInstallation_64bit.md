<!-- Name: OpenSolarisInstallation/64bit -->
<!-- Version: 4 -->
<!-- Last-Modified: 2010/12/08 14:52:04 -->
<!-- Author: springmeyer -->
For the main install page see: https://github.com/mapnik/mapnik/wiki/OpenSolarisInstallation part InstallingCoreMapnikDependencies

## 6b: 64 bit - Postgres 8.3 from sun

NOTE: next step on improving this approach will be to test using Postgres 9.0 and Python >= 2.6 from source, but all 64 bit.

### Getting 64 bit builds
In a best case scenario building a tool 64 bit should be easy if:

1. you can easily pass a few custom CFLAGS and LDFLAGS
2. all the dependencies of that library are also 64 bit and PIC code.

So, sometime it is as easy as:

    export CFLAGS='-m64'
    export CXXFLAGS='-m64'
    export LDFLAGS='-m64 -R/usr/local/lib'
    ./configure
    make
    pfexec make install

But, lots can go wrong, including:

1. code is not properly PIC
2. different compilers are being invoked during configure such that passing those env flags cause the configure to fail
3. older built tools (gcc, libtool, etc) are invoked and flags conflict in 64 bit mode, like -mt, or -M or -tag=CC
4. the built scripts don't properly propogate your custom flags so that some code is built 32 bit and libraries fail with the -m64 flag
5. some built scripts pull env variables from other apps compiled both 32 and 64 bit (like sun provided postgres)
  * in this case you *must* have the 64bit commands on your path, such as PATH=/usr/postgres/bin/amd64


To get started first we set up a few environment variables

```sh
    TARGET="~/.bashrc"
    echo 'export PATH=/usr/bin/amd64:/opt/ts/gcc/4.4/bin/:/opt/ts/bin:/usr/local/bin/:/usr/postgres/8.3/bin/amd64:$PATH' >> $TARGET
    echo 'export PYTHONPATH=/usr/local/lib/python2.6/site-packages:$PYTHONPATH' >>  $TARGET
    source $TARGET
```

Also command like this was needed with the 32 bit approach but appear unneeded with this 64 bit approach:

```
    echo 'export LD_LIBRARY_PATH=/usr/local/lib:/usr/postgres/8.3/lib/amd64' >>  $TARGET
    echo 'export LANG="C"' >>  $TARGET
    echo 'export LC_ALL="C"' >> $TARGET
```

Then set up a build area:

```sh
    # set up a directory for source builds of familiar geo libs
    mkdir src
    export SRC=`pwd`/src
```

Then get on with the installs

```sh
    # icu
    cd $SRC
    wget http://download.icu-project.org/files/icu4c/4.4.1/icu4c-4_4_1-src.tgz
    tar xvf icu4c-4_4_1-src.tgz
    cd icu/source
    export CXX='/opt/ts/gcc/4.4/bin/g++'
    export CC='/opt/ts/gcc/4.4/bin/gcc'
    export CFLAGS='-m64'
    export CXXFLAGS='-m64'
    export LDFLAGS='-m64 -R/usr/local/lib'
    ./runConfigureICU Solaris/GCC --with-library-bits=64
    make
    pfexec make install
```

```sh
    # boost
    cd $SRC
    wget wget http://voxel.dl.sourceforge.net/project/boost/boost/1.45.0/boost_1_45_0.tar.bz2
    tar xjvf boost_1_45_0.tar.bz2
    cd boost_1_45_0
    # create jam file to configure
    echo "using gcc : 4.4 : /opt/ts/gcc/4.4/bin/g++ ; " >> tools/build/v2/user-config.jam
    ./bootstrap.sh
    ./bjam \
      -q \
      -d2 \
      --with-thread \
      --with-filesystem \
      --with-iostreams \
      --with-python \
      --with-program_options \
      --with-system \
      --with-regex -sHAVE_ICU=1 -sICU_PATH=/usr/local  \
      toolset=gcc \
      address-model=64 \
      link=shared \
      release \
      stage
    
    pfexec ./bjam \
      -q \
      -d2 \
      --with-thread \
      --with-filesystem \
      --with-iostreams \
      --with-python \
      --with-program_options \
      --with-system \
      --with-regex -sHAVE_ICU=1 -sICU_PATH=/usr/local  \
      toolset=gcc \
      address-model=64 \
      link=shared \
      release \
      install
```

```sh
    # proj
    cd $SRC
    VER=4.7.0
    wget http://download.osgeo.org/proj/proj-$VER.tar.gz
    wget ftp://ftp.remotesensing.org/proj/proj-datumgrid-1.5.zip
    tar xzf proj-$VER.tar.gz
    cd proj-$VER
    cd nad
    unzip ../../proj-datumgrid-1.5.zip
    cd ..
    export CFLAGS='-m64'
    export CXXFLAGS='-m64'
    export LDFLAGS='-m64 -R/usr/local/lib'
    ./configure
    make
    pfexec make install
```

```sh
    # geos
    cd $SRC
    VER=3.2.0
    wget http://download.osgeo.org/geos/geos-$VER.tar.bz2
    tar xjf geos-$VER.tar.bz2
    cd geos-$VER
    export CFLAGS='-m64'
    export CXXFLAGS='-m64'
    export LDFLAGS='-m64 -R/usr/local/lib'
    ./configure
    make
    pfexec make install
```

For postgres we use the sun provided version (for better or worse):

```
    # install postgres 8.3 from sun
    pfexec pkg install SUNWpostgr-83-server SUNWpostgr-83-devel SUNWpostgr-83-client SUNWpostgr-83-contrib
```

Setup postgres

```
    pfexec mkdir /database/pgdata
    pfexec chown postgres:postgres /database/pgdata
    pfexec su - postgres 
    export PATH=/usr/postgres/8.3/bin/amd64:$PATH
    initdb /database/pgdata
    pg_ctl -D /database/pgdata -l /usr/postgres/data/log start
```

Install postgres (compiled with gcc) against sun provided postgres (likely compiled with ancient suncc version). This is dangerous, but it worked.

```sh
    # postgis
    cd $SRC
    VER=1.5.1
    wget http://postgis.refractions.net/download/postgis-$VER.tar.gz
    tar xvf postgis-$VER.tar.gz
    cd postgis-$VER
    ./configure --with-projdir=/usr/local/
    # this below command could be greatly simplified, but it finally worked so there you go. I think the missing secret was simple `-shared`
    pfexec make CC="gcc -shared -m64 -O2 -fPIC -DPIC" CXX="g++ -shared -m64 -O2 -fPIC -DPIC" CFLAGS="-m64 -O2 -fPIC -DPIC" CXXFLAGS="-m64 -O2 -fPIC -DPIC" CFLAGS_SL='-m64 -O2 -fPIC -DPIC' LDFLAGS_SL='-m64 -O2 -fPIC -DPIC -R/usr/local/lib -R/usr/postgres/8.3/lib/amd64'
    pfexec make install
    # passing -R/usr/local/lib for the runpath still does not allow libpostgis.so to find proj4 and geos when running `psql -f postgis.sql`
    # so symlink them into the same directory as an workaround
    pfexec ln -s /usr/local/lib/libproj.so.0 /usr/postgres/8.3/lib/amd64/libproj.so.0
    pfexec ln -s /usr/local/lib/libgeos_c.so.1 /usr/postgres/8.3/lib/amd64/libgeos_c.so.1
```

The goal with the nasty bit of flags above is to force the proper gcc + 64 bit build environment for postgis to work at runtime. This is tricky because the sun postgres tries to tell postgis to compile using hardcoded paths to compilers that don't even exist for open solaris (due to pg_config) and also because we cannot modify these settings except by overrides to `make` (not configure).

Mainly we are trying to avoid this error:

    psql:/usr/postgres/8.3/share/contrib/postgis-1.5/postgis.sql:59: ERROR:  could not load library "/usr/postgres/8.3/lib/amd64/postgis-1.5.so": ld.so.1: postgres: fatal: relocation error: R_AMD64_PC32: file /usr/postgres/8.3/lib/amd64/postgis-1.5.so: symbol main: value 0x28003ed8ab4 does not fit

Now set up postgis:

```sh
    # create the template_postgis db
    pfexec su - postgres
    export PATH=/usr/local/bin:/usr/postgres/8.3/bin/amd64/:$PATH
    POSTGIS_SQL_PATH=`pg_config --sharedir`/contrib/postgis-1.5
    createdb -E UTF8 template_postgis
    createlang -d template_postgis plpgsql
    psql -q -d template_postgis -f $POSTGIS_SQL_PATH/postgis.sql
    psql -q -d template_postgis -f $POSTGIS_SQL_PATH/spatial_ref_sys.sql
    # now switch back to your normal user
    exit
```

```sh
    # gdal
    cd $SRC
    VER=1.7.2
    wget http://download.osgeo.org/gdal/gdal-$VER.tar.gz
    tar xzf gdal-$VER.tar.gz
    cd gdal-$VER
    export CFLAGS='-m64'
    export CXXFLAGS='-m64'
    export LDFLAGS='-m64 -R/usr/local/lib'
    ./configure --with-proj --with-geos --with-postgres
    make
    pfexec make install
```

```sh
    # osm2pgsql
    cd $SRC
    #svn co http://svn.openstreetmap.org/applications/utils/export/osm2pgsql/
    # grab a hard revision that we know works on solaris (based on wikipedia usage, although they compile with suncc)
    svn co -r 19933 http://svn.openstreetmap.org/applications/utils/export/osm2pgsql/
    cd osm2pgsql
```

Apply patch


```diff
    Index: Makefile
    ===================================================================
    --- Makefile    (revision 19933)
    +++ Makefile    (working copy)
    @@ -22,6 +22,7 @@
     LDFLAGS += -g -lproj
     LDFLAGS += -lstdc++
     LDFLAGS += -lpthread
    +LDFLAGS += -R/usr/postgres/8.3/lib -R/opt/ts/lib -R/usr/local/lib
     
     SRCS:=$(wildcard *.c) $(wildcard *.cpp)
     OBJS:=$(SRCS:.c=.o)
```

finish install

```sh
    make
    pfexec cp osm2pgsql /usr/local/bin
    pfexec chmod +x /usr/local/bin/osm2pgsql
    pfexec mkdir /usr/share/osm2pgsql
    # get latest style and install
    svn up default.style
    pfexec cp default.style /usr/share/osm2pgsql/
```

## Other stuff

Apache, mod_wsgi, tilecache, pil...

    export CFLAGS='-m64'
    export CXXFLAGS='-m64'
    export LDFLAGS='-m64 -R/usr/local/lib'
    
    # apr
    wget http://mirrors.dedipower.com/ftp.apache.org/apr/apr-1.4.2.tar.gz
    
    # apr util
    wget http://apache.ziply.com//apr/apr-util-1.3.10.tar.gz
    tar xvf apr-util-1.3.10.tar.gz
    ./configure --with-apr=/usr/local/apr/bin/apr-1-config
    
    # httpd:
    ./configure --with-apr=/usr/local/apr/bin/apr-1-config --with-apr-util=/usr/local/apr/bin/apu-1-config
    
    # mod_wsgi
    export PATH=/usr/local/apache2/bin/:$PATH
    ./configure --with-apxs=/usr/local/apache2/bin/apxs --with-python=/usr/bin/amd64/python
    make
    pfexec make install
    
    # now start apache
    pfexec /usr/local/apache2/bin/httpd -k start
    

Python PIL Imaging is not built 64 bit via packages (there is no site-packages/PIL/64/_imaging.so) so do:

    # uninstall old version
    pfexec pkg uninstall python-imaging-26
    
    # use the sun tools to compile python apps that use pure c libs:
    export PATH=/opt/sunstudio12.1/bin:/usr/gnu/bin/amd64:/usr/gnu/bin:/usr/bin/amd64:/usr/bin:/bin/amd64:/bin:/usr/sbin/amd64:/usr/sbin:/sbin:/usr/X11/bin/amd64
    CFLAGS="-m64" python setup.py build
    pfexec /usr/bin/amd64/python setup.py install --prefix=/usr/local

ensure:

    $ file /usr/local/lib/python2.6/site-packages/PIL/64/_imaging.so 
    /usr/local/lib/python2.6/site-packages/PIL/64/_imaging.so:      ELF 64-bit LSB dynamic lib AMD64 Version 1, dynamically linked, not stripped, no debugging information available


Also can test PIL is 64 and PIC with:

    import ctypes
    import PIL.Image
    PIL.Image.core.new(4,4)
    ctypes.CDLL('/usr/local/lib/python2.6/site-packages/PIL/64/_imaging.so')

TileCache gocha:

    # now source installed PIL does not support `import Image`, so we change the imports in TC:
    pfexec vim TileCache/Layer.py
    # replace import Image with import PIL.Image as Image

More info: http://trac.osgeo.org/tilecache/ticket/26
