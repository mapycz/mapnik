# Build notes specific to older versions of Ubuntu

See UbuntuInstallation for build notes for the latest version.

# Ubuntu Maverick (10.10)

This release has Mapnik packages for 0.7.1 (to check run `apt-cache show libmapnik*`), so you can either install Mapnik from packages or source.

 * Packages are available in the 'universe' repositories so make sure your `/etc/apt/sources.list` has the below lines (or similar):

```
    deb http://us.archive.ubuntu.com/ubuntu/ maverick universe
    deb http://us.archive.ubuntu.com/ubuntu/ maverick-updates universe
```

### Set up build environment

```sh
    # get a build environment going...
    sudo apt-get install -y g++ cpp \
    libicu-dev \
    libboost-filesystem1.42-dev \
    libboost-iostreams1.42-dev libboost-program-options1.42-dev \
    libboost-python1.42-dev libboost-regex1.42-dev \
    libboost-system1.42-dev libboost-thread1.42-dev \
    python-dev libxml2 libxml2-dev \
    libfreetype6 libfreetype6-dev \
    libjpeg62 libjpeg62-dev \
    libltdl7 libltdl-dev \
    libpng12-0 libpng12-dev \
    libgeotiff-dev libtiff4 libtiff4-dev libtiffxx0c2 \
    libcairo2 libcairo2-dev python-cairo python-cairo-dev \
    libcairomm-1.0-1 libcairomm-1.0-dev \
    ttf-unifont ttf-dejavu ttf-dejavu-core ttf-dejavu-extra \
    subversion build-essential python-nose
    
    # install plugin dependencies
    sudo apt-get install libgdal1-dev python-gdal \
    postgresql-8.4 postgresql-server-dev-8.4 postgresql-contrib-8.4 postgresql-8.4-postgis \
    libsqlite3-dev
```


# Ubuntu Lucid (10.04)

Karmic has Mapnik packages for 0.7.0, so you can either install Mapnik from packages or source.

 * Packages are available in the 'universe' repositories so make sure your `/etc/apt/sources.list` has the below lines:

```
    deb http://us.archive.ubuntu.com/ubuntu/ lucid universe
    deb http://us.archive.ubuntu.com/ubuntu/ lucid-updates universe
```

## Install from packages


```
    sudo apt-get install python-cairo libmapnik0.7 mapnik-utils python-mapnik
```

*Note:* then you will likely want to install Postgres 8.4 + PostGIS 1.4 (see below)

## Install Mapnik from source

### Set up

```sh
    # get a build environment going...
    sudo apt-get install -y g++ cpp \
    libboost1.40-dev libboost-filesystem1.40-dev \
    libboost-iostreams1.40-dev libboost-program-options1.40-dev \
    libboost-python1.40-dev libboost-regex1.40-dev \
    libboost-thread1.40-dev \
    python-dev libxml2 libxml2-dev \
    libfreetype6 libfreetype6-dev \
    libjpeg62 libjpeg62-dev \
    libltdl7 libltdl-dev \
    libpng12-0 libpng12-dev \
    libgeotiff-dev libtiff4 libtiff4-dev libtiffxx0c2 \
    libcairo2 libcairo2-dev python-cairo python-cairo-dev \
    libcairomm-1.0-1 libcairomm-1.0-dev \
    ttf-unifont ttf-dejavu ttf-dejavu-core ttf-dejavu-extra \
    subversion build-essential python-nose
    
    # also a bug in cairomm requires manually grabbing libsigc++:
    # https://bugs.launchpad.net/ubuntu/+source/cairomm/+bug/452733
    sudo apt-get install libsigc++-dev libsigc++0c2 libsigx-2.0-2 libsigx-2.0-dev
    
    # install plugin dependencies
    sudo apt-get install libgdal1-dev python-gdal \
    postgresql-8.4 postgresql-server-dev-8.4 postgresql-contrib-8.4 postgresql-8.4-postgis \
    libsqlite3-dev

```

### Then compile and install Mapnik

For instructions on compiling trunk (aka Mapnik2) see: wiki:Mapnik2

```sh
    svn co http://svn.mapnik.org/tags/release-0.7.1/ mapnik
    cd mapnik
    python scons/scons.py configure INPUT_PLUGINS=all OPTIMIZATION=3 SYSTEM_FONTS=/usr/share/fonts/
    python scons/scons.py
    sudo python scons/scons.py install
```

Then run:

```sh
    $ sudo ldconfig
```

To test mapnik:

```python
    $ Python
    >>> import mapnik
    >>>
```

 * No output is good. 

------------

# Ubuntu Karmic (9.10)

Karmic has Mapnik packages for 0.6.1, so you can either install Mapnik from packages or source.

 * Packages are available in the 'universe' repositories so make sure your `/etc/apt/sources.list` has the below lines:

```
    deb http://us.archive.ubuntu.com/ubuntu/ karmic universe
    deb http://us.archive.ubuntu.com/ubuntu/ karmic-updates universe
```

## Install from packages

```sh
    sudo apt-get install python-cairo libmapnik0.6 mapnik-utils python-mapnik
```

*Note:* then you will likely want to install Postgres 8.4 + PostGIS 1.4 (see below)

## Install Mapnik from source

### Set up

```sh
    # get a build environment going...
    sudo apt-get install -y g++ cpp \
    libboost1.40-dev libboost-filesystem1.40-dev \
    libboost-iostreams1.40-dev libboost-program-options1.40-dev \
    libboost-python1.40-dev libboost-regex1.40-dev \
    libboost-thread1.40-dev \
    libxml2 libxml2-dev \
    libfreetype6 libfreetype6-dev \
    libjpeg62 libjpeg62-dev \
    libltdl7 libltdl-dev \
    libpng12-0 libpng12-dev \
    libgeotiff-dev libtiff4 libtiff4-dev libtiffxx0c2 \
    libcairo2 libcairo2-dev python-cairo python-cairo-dev \
    libcairomm-1.0-1 libcairomm-1.0-dev \
    ttf-dejavu ttf-dejavu-core ttf-dejavu-extra \
    subversion build-essential python-nose
    
    # also a bug in cairomm requires manually grabbing libsigc++:
    # https://bugs.launchpad.net/ubuntu/+source/cairomm/+bug/452733
    sudo apt-get install libsigc++-dev libsigc++0c2 libsigx-2.0-2 libsigx-2.0-dev
    
    # install plugin dependencies
    sudo apt-get install libgdal1-dev python-gdal \
    postgresql-8.4 postgresql-server-dev-8.4 postgresql-contrib-8.4 \
    libsqlite3-dev
    
    # note, the postgis package is not available for postgres-8.4, so install it from source...
    wget http://postgis.refractions.net/download/postgis-1.4.0.tar.gz
    tar xzf postgis-1.4.0.tar.gz
    cd postgis-1.4.0
    ./configure && make && sudo make install
```

### Then compile and install Mapnik

```
    git clone git://github.com/mapnik/mapnik.git
    cd mapnik
    python scons/scons.py configure INPUT_PLUGINS=all OPTIMIZATION=3 SYSTEM_FONTS=/usr/share/fonts/truetype/ttf-dejavu/
    python scons/scons.py
    sudo python scons/scons.py install
```

To test mapnik:

```python
    $ Python
    >>> import mapnik
    >>>
```

 * No output is good. 
If you get errors about missing libs make sure 'usr/local/lib' is in /etc/ld.so.conf:

```sh
    $ more /etc/ld.so.conf
    ## if `/usr/local/lib` is not with that file try:
    $ echo "/usr/local/lib" >> /etc/ld.so.conf
    # ldconfig
```

 * Note: If your system is 64 bit, then /etc/ld.so.conf should include `/usr/local/lib64` instead of /usr/local/lib

```sh
    $ more /etc/ld.so.conf
    ## if `/usr/local/lib` is not with that file try:
    $ echo "/usr/local/lib64" >> /etc/ld.so.conf
    $ sudo ldconfig
```

 * Note: Ubuntu version >= (8.10) should already have `/usr/local/lib` in /etc/ld.so.conf.d/, so try:

```sh
    more /etc/ld.so.conf.d/libc.conf
```

------------

# Ubuntu Jaunty Jackalope (9.04)

This works with the default python-2.6 on jaunty:

```sh
    sudo apt-get install -y g++ cpp \
    libboost1.35-dev libboost-filesystem1.35-dev \
    libboost-iostreams1.35-dev libboost-program-options1.35-dev \
    libboost-python1.35-dev libboost-regex1.35-dev \
    libboost-thread1.35-dev \
    libxml2 libxml2-dev \
    libfreetype6 libfreetype6-dev \
    libjpeg62 libjpeg62-dev \
    libltdl7 libltdl7-dev \
    libpng12-0 libpng12-dev \
    libgeotiff-dev libtiff4 libtiff4-dev \
    libcairo2 libcairo2-dev python-cairo python-cairo-dev \
    libcairomm-1.0-1 libcairomm-1.0-dev \
    ttf-dejavu ttf-dejavu-core ttf-dejavu-extra \
    libgdal1-dev python-gdal \
    postgresql-8.3-postgis postgresql-8.3 \
    postgresql-server-dev-8.3 postgresql-contrib-8.3 \
    libsqlite3-dev  \
    subversion build-essential
    
    git clone git://github.com/mapnik/mapnik.git
    cd mapnik
    python scons/scons.py configure INPUT_PLUGINS=all \
    OPTIMIZATION=3 \
    SYSTEM_FONTS=/usr/share/fonts/truetype/ttf-dejavu/
    python scons/scons.py
    sudo python scons/scons.py install
    sudo ldconfig
```

# Ubuntu Intrepid Ibex (8.10)

Note, these instructions are nearly identical to the [[DebianInstallation]] notes (with the exception of libltdl7/3), just more verbose.

# Prerequisites

## Install all boost dependencies

```sh
    $ sudo apt-get install binutils cpp-3.4 g++ gcc-3.4 gcc-3.4-base \
    libboost-dev libboost-filesystem-dev libboost-filesystem1.34.1 \
    libboost-iostreams-dev libboost-iostreams1.34.1 libboost-program-options-dev \
    libboost-program-options1.34.1 libboost-python-dev libboost-python1.34.1 \
    libboost-regex-dev libboost-regex1.34.1 libboost-serialization-dev \
    libboost-serialization1.34.1 libboost-thread-dev libboost-thread1.34.1 \
    libicu-dev libicu38 libstdc++6 libstdc++6-4.2-dev python2.5-dev
```

## Install all remaining required dependencies

```sh
    $ sudo apt-get install libxml2-dev libxml2 proj libfreetype6 libfreetype6-dev \
    libjpeg62 libjpeg62-dev libltdl7 libltdl7-dev libpng12-0 libpng12-dev \
    libtiff4 libtiff4-dev libtiffxx0c2 python-imaging python-imaging-dbg
```

## Install optional Cairo Renderer dependencies

```sh
    $ sudo apt-get install libcairo2 libcairo2-dev python-cairo python-cairo-dev \
    libcairomm-1.0-1 libcairomm-1.0-dev libglib2.0-0 libpixman-1-0 libpixman-1-dev \
    libpthread-stubs0 libpthread-stubs0-dev ttf-dejavu ttf-dejavu-core ttf-dejavu-extra
```

## Install optional GIS utilities used for plugins

```sh
    $ sudo apt-get install libgdal1-dev python-gdal \
    postgresql-8.3-postgis postgresql-8.3 postgresql-server-dev-8.3 postgresql-contrib-8.3
```

## Install nose for running Mapnik test framework

```sh
    sudo apt-get install python-nose
```

## Ubuntu Hardy Heron (8.04)

## Prerequisites

### Aptitude (or Apt-get) Install all boost dependencies

```sh
    # aptitude install binutils cpp-3.3 g++-3.3 gcc-3.3 gcc-3.3-base libboost-dev libboost-filesystem-dev libboost-filesystem1.34.1 libboost-iostreams-dev libboost-iostreams1.34.1 libboost-program-options-dev libboost-program-options1.34.1 libboost-python-dev libboost-python1.34.1 libboost-regex-dev libboost-regex1.34.1 libboost-serialization-dev libboost-serialization1.34.1 libboost-thread-dev libboost-thread1.34.1 libicu-dev libicu38 libstdc++5 libstdc++5-3.3-dev python2.5-dev 
```

### Aptitude Install all remaining required dependencies

```sh
    # aptitude install libfreetype6 libfreetype6-dev libjpeg62 libjpeg62-dev libltdl3 libltdl3-dev libpng12-0 libpng12-dev libtiff4 libtiff4-dev libtiffxx0c2 python-imaging python-imaging-dbg proj 
```

### Aptitude Install optional Cairo Renderer dependencies

```sh
    # aptitude install libcairo2 libcairo2-dev python-cairo python-cairo-dev libcairomm-1.0-1 libcairomm-1.0-dev libglib2.0-0 libpixman-1-0 libpixman-1-dev libpthread-stubs0 libpthread-stubs0-dev ttf-dejavu ttf-dejavu-core ttf-dejavu-extra
```

### Aptitude Install all Optional GIS utilities

```sh
    # aptitude install libgdal-dev python2.5-gdal postgresql-8.3-postgis postgresql-8.3 postgresql-server-dev-8.3 postgresql-contrib-8.3
```

### Install WMS Dependencies

```sh
    # aptitude install libxslt1.1 libxslt1-dev libxml2-dev libxml2
    # easy_install jonpy
    # easy_install lxml
```
(Note: this requires [EasyInstall](http://peak.telecommunity.com/DevCenter/EasyInstall).)

And install Apache web server if you need it:

```sh
    # aptitude install apache2 apache2-threaded-dev apache2-doc apache2-mpm-prefork apache2-utils 
```

## Build and Install Mapnik

```sh
    $ cd ~/src
    $ svn co http://svn.mapnik.org/tags/release-0.7.0/ mapnik
    $ cd mapnik
    $ python scons/scons.py
    # python scons/scons.py install # hint: use sudo
```

Then run:

```sh
    # ldconfig
```

## Ubuntu Feisty Fawn (7.10)

    TODO

## Ubuntu Gutsy Gibbon (7.04)

_'Note_ : if you experience trouble with boost on 7.04 see this mapnik-devel thread: https://lists.berlios.de/pipermail/mapnik-devel/2008-August/000700.html

### Install all Dependencies

```sh
    # aptitude install cpp-2.95 g++ libboost-dev libboost-filesystem-dev libboost-filesystem1.33.1 libboost-iostreams-dev libboost-iostreams1.33.1 libboost-program-options-dev libboost-program-options1.33.1 libboost-python-dev libboost-python1.33.1 libboost-regex-dev libboost-regex1.33.1 libboost-serialization-dev libboost-thread-dev libboost-thread1.33.1 libc6-dev libicu36-dev libstdc++2.10-dev libstdc++2.10-glibc2.2 linux-libc-dev python2.5-dev libicu36 libicu36-dev libc6-dev linux-libc-dev  libfreetype6 libfreetype6-dev libjpeg62 libjpeg62-dev libltdl3 libltdl3-dev libpng12-0 libpng12-dev libtiff4 libtiff4-dev libtiffxx0c2 python-imaging python-imaging-dbg aptitude install libcairo2 libcairo2-dev python-cairo python-cairo-dev libcairomm-1.0-1 libcairomm-1.0-dev libglib2.0-0 libpixman1 libpixman1-dev libpthread-stubs0 libpthread-stubs0-dev ttf-dejavu proj 
```
### Fix symlinks for Boost libs

```sh
    cd /usr/lib
    ln -s libboost_filesystem.so libboost_filesystem-mt.so
    ln -s libboost_regex.so libboost_regex-mt.so
    ln -s libboost_iostreams.so libboost_iostreams-mt.so
    ln -s libboost_program_options.so libboost_program_options-mt.so
    ln -s libboost_thread.so libboost_thread-mt.so
    ln -s libboost_python.so libboost_python-mt.so
```

### Install all Optional GIS utilities

```sh
    install libgdal1-1.3.2-dev python-gdal postgresql-8.1-postgis postgresql-8.1 postgresql-server-dev-8.1 postgresql-contrib-8.1
```