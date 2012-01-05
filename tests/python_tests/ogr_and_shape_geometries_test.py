#!/usr/bin/env python

from nose.tools import *

from utilities import execution_path, Todo

import os, sys, glob, mapnik

def setup():
    # All of the paths used are relative, if we run the tests
    # from another directory we need to chdir()
    os.chdir(execution_path('.'))

# TODO - fix truncation in shapefile...
polys = ["POLYGON ((30 10, 10 20, 20 40, 40 40, 30 10))",
         "POLYGON ((35 10, 10 20, 15 40, 45 45, 35 10),(20 30, 35 35, 30 20, 20 30))",
         "MULTIPOLYGON (((30 20, 10 40, 45 40, 30 20)),((15 5, 40 10, 10 20, 5 10, 15 5)))"
         "MULTIPOLYGON (((40 40, 20 45, 45 30, 40 40)),((20 35, 45 20, 30 5, 10 10, 10 30, 20 35),(30 20, 20 25, 20 15, 30 20)))"
        ]

plugins = mapnik.DatasourceCache.instance().plugin_names()
if 'shape' in plugins and 'ogr' in plugins:

    def test_geometries_are_interpreted_equivalently():
        shapefile = '../data/shp/wkt_poly.shp'
        ds1 = mapnik.Ogr(file=shapefile,layer_by_index=0)
        ds2 = mapnik.Shapefile(file=shapefile)
        fs1 = ds1.featureset()
        fs2 = ds2.featureset()
        raise Todo("output will differ between ogr and shape, may not matter, needs a closer look")
        count = 0;
        while(True):
            count += 1
            feat1 = fs1.next()
            feat2 = fs2.next()
            if not feat1:
                break
            #import pdb;pdb.set_trace()
            #print feat1
            eq_(str(feat1),str(feat2))
            eq_(feat1.geometries().to_wkt(),feat2.geometries().to_wkt())

if __name__ == "__main__":
    setup()
    [eval(run)() for run in dir() if 'test_' in run]
