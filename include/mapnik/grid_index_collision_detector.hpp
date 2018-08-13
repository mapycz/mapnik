#pragma once

#include <mapnik/grid_index.hpp>
#include <mapnik/value_types.hpp>

namespace mapnik {

class grid_index_collision_detector
{
    const int cells_count = 16;
public:
    grid_index_collision_detector(box2d<double> const& extent);
    bool has_placement(box2d<double> const& box) const;
    bool has_placement(box2d<double> const& box, double margin) const;
    bool has_placement(box2d<double> const& box,
                       double margin,
                       mapnik::value_unicode_string const& text,
                       double repeat_distance) const;
    void insert(box2d<double> const& box);
    void insert(box2d<double> const& box,
                mapnik::value_unicode_string const& text);
    void clear();
    box2d<double> const& extent() const;

    using grid_index_type = grid_index<mapnik::value_unicode_string>;
    using element_type = grid_index_type::element;

    grid_index_type::elements_iterator begin() const;
    grid_index_type::elements_iterator end() const;

private:
    box2d<double> extent_;
    grid_index_type grid_;

#ifdef MAPNIK_STATS_RENDER
public:
    mutable unsigned long query_count_;

    inline int count_items() const
    {
        return grid_.size();
    }
#endif
};

} // namespace mapnik
