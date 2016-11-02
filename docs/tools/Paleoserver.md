<!-- Name: Paleoserver -->
<!-- Version: 2 -->
<!-- Last-Modified: 2011/04/28 09:44:27 -->
<!-- Author: manelclos -->

### Overview
Paleoserver is a new asynchronous C++ wms for mapnik that leverages the latest boost libraries (boost::asio and boost::spirit) for multi-threaded rendering and request parsing.

It requires at least Boost 1.42 and Mapnik 0.7.2 (ideally Mapnik trunk).

It currently only supports being compiled with g++ on linux and mac osx (windows support is planned).

It is in heavy development. For applications needing fully featured Mapnik WMS you should still use the pure python-based *ogcserver* (https://github.com/mapnik/OGCServer).

### FAQ

*Q:* It is fast?

*A:* It is proving to scale very well under high load when launched with many allowable threads. We look forward to providing some detailed benchmarks in the near future about speed of the server vs. other WMS alternatives.

-----

*Q:* Why a new server?

*A:* Several reasons: 1) Mapnik has always needed a pure C++ server that could be faster than the pure-python implementation and the FOSS4G 2010 performance benchmark was a good motivator for starting this project now, and 2) Dane Springmeyer (the author) was looking for a good environment to test multi-threaded rendering pipelines (and ways that Mapnik core might improve or become more flexible) and a HTTP server in boost::asio was a great way to achieve and test this.

-----

*Q:* Does the Paleoserver replace the ogcserver?

*A:* No, at this time Dane plans to continue to develop and maintain the python *ogcserver* WMS implementation (to the extent there is time in the day!). Hopefully in the long term lessons learned from the C++ version may improve design ideas around the ogcserver.

### Code

For now the code can be downloaded from github: http://github.com/springmeyer/paleoserver

```sh
    $ git clone git@github.com:springmeyer/paleoserver.git
```

### Building

The build scripts are not sophisticated at this point and do not accept options from the command line yet.

So, basically you need to open up 'SConstruct' and add any custom paths to libraries that Mapnik needs (eg. freetype, boost, etc).

The build requires an external installation of SCons:

```sh
    $ sudo easy_install scons
```

Then to compile do:

```sh
    $ scons
```