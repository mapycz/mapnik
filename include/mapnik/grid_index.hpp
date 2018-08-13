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
 rectangles in a 2d plane.
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
    using bbox_type = box2d<double>;

    struct element
    {
        element(bbox_type const& box, T const& id)
            : box(box), id(id)
        {
        }

        bbox_type box;
        T id;
    };

    using elements_type = std::vector<element>;
    using elements_iterator = typename std::vector<element>::const_iterator;

    grid_index(const double width_, const double height_, const int16_t cellSize_);

    void insert(T const& t, const bbox_type&);
    void clear();
    
    std::vector<T> query(const bbox_type&) const;
    std::vector<element> queryWithBoxes(const bbox_type&) const;
    
    bool hitTest(const bbox_type&) const;
    
    bool empty() const;
    std::size_t size() const;

    elements_iterator begin() const;
    elements_iterator end() const;

private:
    bool noIntersection(const bbox_type& queryBBox) const;
    bool completeIntersection(const bbox_type& queryBBox) const;

    void query(const bbox_type&, std::function<bool (element const&)>) const;

    int16_t convertToXCellCoord(const double x) const;
    int16_t convertToYCellCoord(const double y) const;
    
    bool boxesCollide(const bbox_type&, const bbox_type&) const;

    const double width;
    const double height;
    
    const int16_t xCellCount;
    const int16_t yCellCount;
    const double xScale;
    const double yScale;

    std::vector<element> boxElements;
    
    std::vector<std::vector<size_t>> boxCells;
};

} // namespace mapnik
