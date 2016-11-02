# Installing Mapnik on Ubuntu

For all versions of Ubuntu it is a good idea to be fully up to date before starting:

```sh
sudo apt-get update
sudo apt-get upgrade
```

For older versions, see the archived notes at [[UbuntuInstallationOld]]

If you are intending to install [Tilemill](http://mapbox.com/tilemill/) from a package/ppa, do not follow the directions below in order to avoid package conflicts. Instead, directly proceed to [install Tilemill](http://mapbox.com/tilemill/docs/linux-install/). Mapnik will automatically be installed with your Tilemill installation because it is available in the [TileMill PPA](https://launchpad.net/~developmentseed/+archive/mapbox/). 

# Ubuntu (16.04)

Install Mapnik latest (3.x series)

```
git clone https://github.com/mapnik/mapnik mapnik-3.x --depth 10
cd mapnik-3.x
git submodule update --init
sudo apt-get install python zlib1g-dev clang make pkg-config curl
source bootstrap.sh
./configure CUSTOM_CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0"
make
make test
sudo make install
```

----

# Ubuntu >= (11.10) and < (14.04)

## Install from packages

First, ensure `add-apt-repository` is installed:

```sh
sudo apt-get install -y python-software-properties
```

**Mapnik v2.2.0**

This is the latest in the 2.2.x series.

```sh
sudo add-apt-repository ppa:mapnik/v2.2.0
sudo apt-get update
sudo apt-get install libmapnik libmapnik-dev mapnik-utils python-mapnik
```

These packages come from: https://launchpad.net/~mapnik/+archive/v2.2.0/+packages

**Mapnik v2.3.x**

This is the nightly build of the upcoming 2.3.0 release

```sh
sudo add-apt-repository ppa:mapnik/nightly-2.3
sudo apt-get update
sudo apt-get install libmapnik libmapnik-dev mapnik-utils python-mapnik
# also install datasource plugins if you need them
sudo apt-get install mapnik-input-plugin-gdal mapnik-input-plugin-ogr\
  mapnik-input-plugin-postgis \
  mapnik-input-plugin-sqlite \
  mapnik-input-plugin-osm
```

These packages come from: https://launchpad.net/~mapnik/+archive/nightly-2.3/+packages

**For nightly builds from master (3.0.0-pre)**

This is the latest development code - built nightly - directly from https://github.com/mapnik/mapnik/commits/master

```sh
sudo add-apt-repository ppa:mapnik/nightly-trunk
sudo apt-get update
sudo apt-get install libmapnik libmapnik-dev mapnik-utils python-mapnik
sudo apt-get install mapnik-input-plugin-gdal mapnik-input-plugin-ogr\
  mapnik-input-plugin-postgis \
  mapnik-input-plugin-sqlite \
  mapnik-input-plugin-osm
```

These packages come from: https://launchpad.net/~mapnik/+archive/nightly-trunk/+packages

## Install Mapnik from source

First, remove any other old mapnik packages:

```sh
sudo apt-get purge libmapnik* mapnik-* python-mapnik
```

### Ensure your boost version is recent enough (at least 1.47)

Mapnik master may require a boost version more recent than provided by your Ubuntu distribution. 

Ubuntu 12.04 Precise ships with 2 different boost versions: 1.46 and 1.48. Make sure you install the correct version (see below) or use the latest Boost version (that works with Mapnik) by installing Boost from the `mapnik/boost` PPA:

```sh
sudo add-apt-repository ppa:mapnik/boost
sudo apt-get update
sudo apt-get install libboost-dev libboost-filesystem-dev libboost-program-options-dev libboost-python-dev libboost-regex-dev libboost-system-dev libboost-thread-dev 
```
Note: You can see the boost version offered by your distro with the below command. And if you are using the above PPA then its version should show up as a candidate for installation:

```sh
apt-cache policy libboost-dev
```

### Set up build environment

```sh
    # On Ubuntu 12.04 Precise, make sure you get the 1.48 boost packages:
    sudo apt-get install \
    libboost-filesystem1.48-dev \
    libboost-program-options1.48-dev \
    libboost-python1.48-dev libboost-regex1.48-dev \
    libboost-system1.48-dev libboost-thread1.48-dev

    # On newer system or if you've activated the mapnik PPA, then use this:
    sudo apt-get install \
    libboost-filesystem-dev \
    libboost-program-options-dev \
    libboost-python-dev libboost-regex-dev \
    libboost-system-dev libboost-thread-dev \

    # get a build environment going...
    sudo apt-get install \
    libicu-dev \
    python-dev libxml2 libxml2-dev \
    libfreetype6 libfreetype6-dev \
    libjpeg-dev \
    libpng-dev \
    libproj-dev \
    libtiff-dev \
    libcairo2 libcairo2-dev python-cairo python-cairo-dev \
    libcairomm-1.0-1 libcairomm-1.0-dev \
    ttf-unifont ttf-dejavu ttf-dejavu-core ttf-dejavu-extra \
    git build-essential python-nose \
    libgdal1-dev python-gdal \
    postgresql-9.1 postgresql-server-dev-9.1 postgresql-contrib-9.1 postgresql-9.1-postgis \
    libsqlite3-dev
```

Note: for Ubuntu >=14.04 postgres/postgis versions have shifted, so do:

```
sudo apt-get install -y postgresql-9.3 postgresql-server-dev-9.3 postgresql-contrib-9.3 postgresql-9.3-postgis-2.1
```

### Source install of Mapnik 2.3.x

```sh
# For the development branch:
git clone https://github.com/mapnik/mapnik mapnik-2.3.x -b 2.3.x --depth 10
cd mapnik-2.3.x
./configure && make && sudo make install
```

### Source install of Mapnik Master (3.x)

First download, compile and install harfbuzz
```sh
wget http://www.freedesktop.org/software/harfbuzz/release/harfbuzz-0.9.26.tar.bz2
tar xf harfbuzz-0.9.26.tar.bz2
cd harfbuzz-0.9.26
./configure && make && sudo make install
sudo ldconfig
cd ../
```

Upgrade your compiler to at least g++ 4.7 so it supports c++11 features. <br>`apt-get upgrade` should give you g++-4.8 and gcc-4.8):
```sh
apt-get update
apt-get upgrade
git clone https://github.com/mapnik/mapnik --depth 10
cd mapnik
git submodule update --init deps/mapbox/variant
./configure
make && sudo make install
```

If that doesn't work, here is an example for ubuntu precise to upgrade compilers:

```sh
CLANG_VERSION=3.6
sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test;
sudo add-apt-repository "deb http://llvm.org/apt/precise/ llvm-toolchain-precise-${CLANG_VERSION} main";
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key|sudo apt-key add -
sudo apt-get update -y
sudo apt-get install -y clang-3.6;
export CXX="clang++-3.6" && export CC="clang-3.6";
git clone https://github.com/mapnik/mapnik --depth 10
cd mapnik
./configure CXX=${CXX} CC=${CC}
make && sudo make install
```





### Testing

To test mapnik:

```sh
make test
```