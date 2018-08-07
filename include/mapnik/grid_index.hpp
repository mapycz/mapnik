#pragma once

#include <mapnik/geometry.hpp>
#include <mapnik/box2d.hpp>

#include <cstdint>
#include <cstddef>
#include <vector>
#include <functional>

namespace mapnik {

/*
 grid_index is a data structure for testing the intersection of
 circles and rectangles in a 2d plane.
 It is optimized for rapid insertion and querying.
 grid_index splits the plane into a set of "cells" and keeps track
 of which geometries intersect with each cell. At query time,
 full geometry comparisons are only done for items that share
 at least one cell. As long as the geometries are relatively
 uniformly distributed across the plane, this greatly reduces
 the number of comparisons necessary.
*/

template <class T>
class MAPNIK_DECL grid_index {
public:

    grid_index(const double width_, const double height_, const int16_t cellSize_);

    using BBox = box2d<double>;

    void insert(T&& t, const BBox&);
    
    std::vector<T> query(const BBox&) const;
    std::vector<std::pair<T,BBox>> queryWithBoxes(const BBox&) const;
    
    bool hitTest(const BBox&) const;
    
    bool empty() const;

private:
    bool noIntersection(const BBox& queryBBox) const;
    bool completeIntersection(const BBox& queryBBox) const;

    void query(const BBox&, std::function<bool (const T&, const BBox&)>) const;

    int16_t convertToXCellCoord(const double x) const;
    int16_t convertToYCellCoord(const double y) const;
    
    bool boxesCollide(const BBox&, const BBox&) const;

    const double width;
    const double height;
    
    const int16_t xCellCount;
    const int16_t yCellCount;
    const double xScale;
    const double yScale;

    std::vector<std::pair<T, BBox>> boxElements;
    
    std::vector<std::vector<size_t>> boxCells;
};

} // namespace mapnik
