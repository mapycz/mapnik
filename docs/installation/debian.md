# Install Mapnik on Debian

## Install mapnik 2 from packages

**Wheezy**:

Version 2.0 Directly bundled in distribution

```sh
apt-get install libmapnik2-2.0 mapnik-utils
```

**Squeeze**:

A backport was done from Wheezy version

Follow instructions from http://osm.fsffrance.org/debian-backports/README to install

# Dependencies for Debian 7: Wheezy (stable) for Mapnik 2.2

```sh
    sudo apt-get install \
    g++ cpp \
    libicu-dev libicu48 \
    python-dev \
    libboost-system-dev \
    libboost-filesystem-dev \
    libboost-iostreams-dev \
    libboost-thread-dev \
    libboost-python-dev \
    libboost-program-options-dev \
    libboost-regex-dev \
    libxml2 libxml2-dev \
    libfreetype6 libfreetype6-dev \
    libjpeg8 libjpeg8-dev \
    libpng12-0 libpng12-dev \
    libtiff5 libtiff5-dev \
    libltdl7 libltdl-dev \
    libproj0 libproj-dev \
    libcairo2 libcairo2-dev python-cairo python-cairo-dev \
    libcairomm-1.0-1 libcairomm-1.0-dev \
    ttf-dejavu ttf-dejavu-core ttf-dejavu-extra ttf-unifont \
    postgresql postgresql-server-dev-9.1 postgresql-contrib \
    libgdal1-dev python-gdal \
    postgresql-9.1-postgis libsqlite3-dev  \
    subversion build-essential python-nose
```

# Dependencies for Debian 6: Squeeze (stable)

```sh
    sudo apt-get install \
    g++-4.4 cpp \
    libicu-dev libicu44 \
    python2.6-dev \
    libboost-system1.42-dev \
    libboost-filesystem1.42-dev \
    libboost-iostreams1.42-dev \
    libboost-thread1.42-dev \
    libboost-python1.42-dev \
    libboost-program-options1.42-dev \
    libboost-regex1.42-dev \
    libxml2 libxml2-dev \
    libfreetype6 libfreetype6-dev \
    libjpeg62 libjpeg62-dev \
    libpng12-0 libpng12-dev \
    libtiff4 libtiff4-dev \
    libltdl7 libltdl-dev \
    libproj0 libproj-dev \
    libcairo2 libcairo2-dev python-cairo python-cairo-dev \
    libcairomm-1.0-1 libcairomm-1.0-dev \
    ttf-dejavu ttf-dejavu-core ttf-dejavu-extra ttf-unifont \
    postgresql-8.4 postgresql-server-dev-8.4 postgresql-contrib-8.4 \
    libgdal1-dev python-gdal \
    postgresql-8.4-postgis libsqlite3-dev  \
    subversion build-essential python-nose
```

*Note:* We use libjpeg62 instead of libjpeg8 above because libtiff package still depends on 62.


# Dependencies for Debian 5: Lenny


```sh
    sudo apt-get install -y g++ cpp \
    libboost-system1.35-dev \
    libboost-filesystem1.35-dev \
    libboost-iostreams1.35-dev \
    libboost-thread1.35-dev \
    libboost-python1.35-dev \
    libboost-program-options1.35-dev \
    libboost-regex1.35-dev \
    libxml2 libxml2-dev \
    libfreetype6 libfreetype6-dev \
    libjpeg62 libjpeg62-dev \
    libltdl3 libltdl3-dev \
    libpng12-0 libpng12-dev \
    libgeotiff-dev libtiff4 libtiff4-dev \
    libcairo2 libcairo2-dev python-cairo python-cairo-dev \
    libcairomm-1.0-1 libcairomm-1.0-dev \
    ttf-dejavu ttf-dejavu-core ttf-dejavu-extra \
    libgdal1-dev python-gdal \
    postgresql-8.3-postgis postgresql-8.3 \
    postgresql-server-dev-8.3 postgresql-contrib-8.3 \
    libsqlite3-dev  \
    build-essential python-nose
```

Mapnik trunk:

```sh
    git clone git://github.com/mapnik/mapnik.git mapnik-trunk
    cd mapnik-trunk
    python scons/scons.py configure INPUT_PLUGINS=all \
    OPTIMIZATION=3 \
    SYSTEM_FONTS=/usr/share/fonts/truetype/
    python scons/scons.py
    sudo python scons/scons.py install
    sudo ldconfig
```

## Optional: Build and install mapnik without root permissions
The development libraries have to be present as described above. In the following example `/home/$USER/mapnik_svn` (`$DIR_MAPNIK_SVN`) is the directory where you want to check out the source and `/home/$USER/mapnik_inst` (`$DIR_MAPNIK_INSTALL`) is the directory you want to use for installation. 

```sh
    DIR_MAPNIK_SRC=/home/$USER/mapnik_svn
    DIR_MAPNIK_INSTALL=/home/$USER/mapnik_inst
    
    git clone git://github.com/mapnik/mapnik.git $DIR_MAPNIK_SVN
    cd $DIR_MAPNIK_SVN
    
    python scons/scons.py configure INPUT_PLUGINS=all \
    OPTIMIZATION=3 \
    SYSTEM_FONTS=/usr/share/fonts/truetype/ttf-dejavu/ \
    PREFIX=$DIR_MAPNIK_INSTALL \
    PYTHON_PREFIX=$DIR_MAPNIK_INSTALL
    python scons/scons.py
    python scons/scons.py install
    
    # append the following lines to /home/$USER/.bashrc
    DIR_MAPNIK_INSTALL=/home/$USER/mapnik_inst
    export LD_LIBRARY_PATH=$DIR_MAPNIK_INSTALL/lib
    export PYTHONPATH=$DIR_MAPNIK_INSTALL/lib/python2.6/site-packages
```
