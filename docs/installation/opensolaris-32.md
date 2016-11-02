<!-- Name: OpenSolarisInstallation/32bit -->
<!-- Version: 2 -->
<!-- Last-Modified: 2010/12/08 13:45:47 -->
<!-- Author: springmeyer -->
For the main install page see: https://github.com/mapnik/mapnik/wiki/OpenSolarisInstallation part InstallingCoreMapnikDependencies

## 6a: 32 bit - Postgres 8.4 from source

First we set up a few environment variables

```sh
    TARGET="~/.bashrc"
    echo 'export PATH=/opt/ts/gcc/4.4/bin/:/opt/ts/bin:/usr/local/bin/:/usr/local/pgsql/bin:$PATH' >> $TARGET
    echo 'export PYTHONPATH=/usr/local/lib/python2.6/site-packages:$PYTHONPATH' >>  $TARGET
    echo 'export LD_LIBRARY_PATH=/usr/local/lib/:/usr/local/pgsql/lib/' >>  $TARGET
    echo 'export LANG="C"' >>  $TARGET
    echo 'export LC_ALL="C"' >> $TARGET
    source $TARGET
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
    ./runConfigureICU Solaris/GCC
    make
    pfexec make install
    
    # boost
    cd $SRC
    wget wget http://voxel.dl.sourceforge.net/project/boost/boost/1.44.0/boost_1_44_0.tar.bz2
    tar xjvf boost_1_44_0.tar.bz2
    cd boost_1_44_0
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
      link=shared \
      release \
      install
    
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
    ./configure
    make
    pfexec make install
    
    # geos
    cd $SRC
    VER=3.2.0
    wget http://download.osgeo.org/geos/geos-$VER.tar.bz2
    tar xjf geos-$VER.tar.bz2
    cd geos-$VER
    ./configure
    make
    pfexec make install
    
    # postgresql
    cd $SRC
    VER=8.4.4
    wget http://wwwmaster.postgresql.org/redir/198/h/source/v$VER/postgresql-$VER.tar.gz
    tar xzvf postgresql-$VER.tar.gz
    cd postgresql-$VER
    ./configure --enable-thread-safety
    make 
    pfexec make install
    pfexec mkdir /usr/local/pgsql/data
    pfexec chown postgres.postgres /usr/local/pgsql/data
    pfexec su - postgres 
    /usr/local/pgsql/bin/initdb /usr/local/pgsql/data/ 
    # Run a postgres instance
    /usr/local/pgsql/bin/postgres -D /usr/local/pgsql/data/
    
    # postgis
    cd $SRC
    VER=1.5.1
    wget http://postgis.refractions.net/download/postgis-$VER.tar.gz
    tar xvf postgis-$VER.tar.gz
    cd postgis-$VER
    ./configure --with-projdir=/usr/local/
    make
    pfexec make install
    
    # create symlinks so that libproj and libgeos can be found be postgis.so
    # we need to do this as their appears to be no known way to pass -R/usr/local/lib
    # to postgres or postgis at compile time
    pfexec ln -s /usr/local/lib/libproj.so.0 /usr/local/pgsql/lib/libproj.so.0
    pfexec ln -s /usr/local/lib/libgeos_c.so.1 /usr/local/pgsql/lib/libgeos_c.so.1
    
    # create the template_postgis db
    pfexec su - postgres
    export PATH=/usr/local/bin:/usr/local/pgsql/bin/:$PATH
    export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/pgsql/lib
    POSTGIS_SQL_PATH=`pg_config --sharedir`/contrib/postgis-1.5
    createdb -E UTF8 template_postgis
    createlang -d template_postgis plpgsql
    psql -q -d template_postgis -f $POSTGIS_SQL_PATH/postgis.sql
    psql -q -d template_postgis -f $POSTGIS_SQL_PATH/spatial_ref_sys.sql
    # now switch back to your normal user
    exit
    
    
    # gdal
    cd $SRC
    VER=1.7.2
    wget http://download.osgeo.org/gdal/gdal-$VER.tar.gz
    tar xzf gdal-$VER.tar.gz
    cd gdal-$VER
    ./configure --with-proj --with-geos --with-postgres
    make
    pfexec make install


    # osm2pgsql
    cd $SRC
    #svn co http://svn.openstreetmap.org/applications/utils/export/osm2pgsql/
    # grab a hard revision that we know works on solaris (based on wikipedia usage, although they compile with suncc)
    svn co -r 19933 http://svn.openstreetmap.org/applications/utils/export/osm2pgsql/
    cd osm2pgsql
```
apply patch

```diff
    Index: Makefile
    ===================================================================
    --- Makefile    (revision 19933)
    +++ Makefile    (working copy)
    @@ -22,6 +22,7 @@
     LDFLAGS += -g -lproj
     LDFLAGS += -lstdc++
     LDFLAGS += -lpthread
    +LDFLAGS += -R/usr/local/pgsql/lib -R/opt/ts/lib -R/usr/local/lib
     
     SRCS:=$(wildcard *.c) $(wildcard *.cpp)
     OBJS:=$(SRCS:.c=.o)
```

finish install

```
    make
    pfexec cp osm2pgsql /usr/local/bin
    pfexec chmod +x /usr/local/bin/osm2pgsql
    pfexec mkdir /usr/share/osm2pgsql
    # get latest style and install
    svn up default.style
    pfexec cp default.style /usr/share/osm2pgsql/
```