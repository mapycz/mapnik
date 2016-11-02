<!-- Name: ModServer -->
<!-- Version: 2 -->
<!-- Last-Modified: 2008/11/23 18:15:43 -->
<!-- Author: tom -->
ModServer is an experimental mod_python driver, just like the cgi and wsgi drivers, etc., for Mapnik.

## Summary

Mainly it was developed because Mapnik was missing a mod_python-based WMS server. Internally, it's extremely similar to cgiserver.py, and externally, it is identical - as far as WMS compatibility, etc.

Modserver isn't meant to be a replacement for TileCache - it is quite different in a few ways:

 * Modserver, like cgiserver, etc., supports WMS, not WMS-C
 * Modserver doesn't have any caching mechanism built in. [We're](http://www.developmentseed.org/) using it with a lightweight PHP cache layer - something Python-based or something that uses memcached is a possibility but right now just plug in what you have.

Currently the code is available on a [Git ticket here.](https://github.com/mapnik/mapnik/issues/101) A well-polished patch with error handling is in the works.