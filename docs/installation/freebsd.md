# Installing on FreeBSD

In most cases the FreeBSD port of Mapnik (located in /usr/ports/graphics/mapnik) will work. However, if the port is broken, or you'd like to install another version the following instructions will help.

You will need to install the following ports/packages to ensure that mapnik will compile for you:

    graphics/png
    graphics/tiff
    graphics/jpeg
    graphics/proj
    devel/icu
    print/freetype2
    graphics/cairo
    graphics/cairomm
    print/harfbuzz
    devel/pkg-config
    graphics/py-cairo
    devel/boost-python-libs
    devel/libtool22
    devel/libltdl22

There may be other ports/packages that need installing

If you want postgres support, ensure that the postgres libs are installed:

    database/postgresqlXX-client (where XX is the version of Postgres libs you'd like to install)

Once these libs are installed, you should be able to compile/install source by doing:

```sh
./configure HB_INCLUDES=/usr/local/include HB_LIBS=/usr/local/lib 
gmake
gmake install
```

## Other references

 - discussion of installing TileMill: http://support.mapbox.com/discussions/tilemill/2776-tilemill-installation-on-freebsd#comment_21576061
 - https://gist.github.com/springmeyer/fabd05d5535e086d5d51
