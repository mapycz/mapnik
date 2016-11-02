<!-- Name: OpenSuseInstallation -->
<!-- Version: 4 -->
<!-- Last-Modified: 2010/09/29 07:15:24 -->
<!-- Author: gnijholt -->


# Installing Mapnik on Suse

One old guide for trunk is here:  http://lists.berlios.de/pipermail/mapnik-users/2010-January/002810.html

But you should install the latest Mapnik release as trunk requires a more recent version of boost.


## OpenSUSE: Installing Mapnik dependencies
TODO

## OpenSUSE: Installing Mapnik
TODO

## SLES 11 x86_64: Installing Mapnik dependencies

 * Mount the SDK ISO number 1 (SLE-11-SDK-DVD-x86_64-GM-Media1.iso)

 * In YaST2, make sure to enable "Search in" -> "File list" and search for "gcc".

 * Make sure these packages are checked (otherwise Boost will not be found during the scons configure stage of Mapnik installation):

  * 'boost-devel'
  * 'cpp43'
  * 'gcc'
  * 'gcc43'
  * 'gcc43-c++'
  * 'gcc-c++'
  * 'libgcc43'

 * As root, issue the following:

```
    $ zypper ar http://download.opensuse.org/repositories/Application:/Geo/SLE_11/ "Geo"
    $ zypper refresh
    $ zypper install libjpeg-devel libtiff-devel libpng-devel boost-devel python-cairo-devel 
    $ zypper install cairomm-devel libicu-devel libtool libxml2-devel libproj0 libproj-devel 
    $ zypper install subversion
```

## SLES 11 x86_64: Installing Mapnik
Still as root:

```
    $ cd && mkdir src && cd src
    $ git clone git://github.com/mapnik/mapnik.git
    $ cd mapnik/
    $ python scons/scons.py configure
    $ python scons/scons.py install
    $ ldconfig
```

 * Next, start the python REPL and try to import mapnik:

```sh
    $ python
```
```python
    >>> import mapnik
    >>> mapnik.mapnik_version()
    701
    >>>
```