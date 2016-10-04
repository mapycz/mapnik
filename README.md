
# mapy.cz-mapnik

This repository contains a fork of [github.com/mapnik/mapnik](https://github.com/mapnik/mapnik), used mostly for rendering maps on [mapy.cz](https://mapy.cz). Main differences against upstream are:

* [Tree-like layer structure](docs/features/layer-structure.md)
* [Layer level compositing](docs/features/layer-level-compositing.md)
* [Text placement method `combined`](docs/features/text-placement-combined.md)
* [Text placement method `angle`](docs/features/text-placement-angle.md)
* [Anchors](docs/features/anchors.md)
* [Text placement `grid`](docs/features/text-placement-grid.md)
* [Text on extended geometry](docs/features/text-extend.md)
* [Shadows with building symbolizer](docs/features/building-symbolizer-shadow.md)
* Offsetting algorithm
* Constant height of a first text line
* [XSLT preprocessing](docs/features/xslt-preprocessing.md)

# Mapnik

Mapnik is an open source toolkit for developing mapping applications. At the core is a C++ shared library providing algorithms and patterns for spatial data access and visualization.

Mapnik is basically a collection of geographic objects like maps, layers, datasources, features, and geometries. The library doesn't rely on any OS specific "windowing systems" and it can be deployed to any server environment. It is intended to play fair in a multi-threaded environment and is aimed primarily, but not exclusively, at web-based development.

For further information see [http://mapnik.org](http://mapnik.org) and also our [wiki documentation](https://github.com/mapnik/mapnik/wiki).

# Installation

See [INSTALL.md](INSTALL.md) for installation instructions and the [Install](https://github.com/mapnik/mapnik/wiki/Mapnik-Installation) page on the wiki for guides.

# License

Mapnik software is free and is released under the LGPL ([GNU Lesser General Public License](http://www.gnu.org/licenses/lgpl.html)). Please see [COPYING](https://github.com/mapnik/mapnik/blob/master/COPYING) for more information.
