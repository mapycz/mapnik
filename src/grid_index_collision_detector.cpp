#include <mapnik/grid_index_collision_detector.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>
#pragma GCC diagnostic pop

namespace mapnik {


grid_index_collision_detector::grid_index_collision_detector(
    box2d<double> const& extent)
    : extent_(extent),
      grid_(extent.width(), extent.height(),
            std::min(extent.width(), extent.height()) / cells_count)
#ifdef MAPNIK_STATS_RENDER
      , query_count_(0)
#endif
{

}

bool grid_index_collision_detector::has_placement(
    box2d<double> const& box) const
{
#ifdef MAPNIK_STATS_RENDER
    ++query_count_;
#endif
    return !grid_.hitTest(box);
}

bool grid_index_collision_detector::has_placement(
    box2d<double> const& box,
    double margin) const
{
#ifdef MAPNIK_STATS_RENDER
    ++query_count_;
#endif

    box2d<double> const& margin_box = (margin > 0
        ? box2d<double>(box.minx() - margin, box.miny() - margin,
            box.maxx() + margin, box.maxy() + margin)
        : box);
    return !grid_.hitTest(margin_box);
}

bool grid_index_collision_detector::has_placement(
    box2d<double> const& box,
    double margin, mapnik::value_unicode_string const& text,
    double repeat_distance) const
{
#ifdef MAPNIK_STATS_RENDER
    ++query_count_;
#endif

    // Don't bother with any of the repeat checking unless
    // the repeat distance is greater than the margin
    if (repeat_distance <= margin)
    {
        return has_placement(box, margin);
    }

    box2d<double> repeat_box(
        box.minx() - repeat_distance,
        box.miny() - repeat_distance,
        box.maxx() + repeat_distance,
        box.maxy() + repeat_distance);

    box2d<double> const& margin_box = (margin > 0
        ? box2d<double>(box.minx() - margin, box.miny() - margin,
            box.maxx() + margin, box.maxy() + margin)
        : box);

    auto elements = grid_.queryWithBoxes(repeat_box);

    for (auto const& element : elements)
    {
        if (element.box.intersects(margin_box) || element.id == text)
        {
            return false;
        }
    }

    return true;
}

void grid_index_collision_detector::insert(
    box2d<double> const& box)
{
    grid_.insert(mapnik::value_unicode_string(), box);
}

void grid_index_collision_detector::insert(
    box2d<double> const& box,
    mapnik::value_unicode_string const& text)
{
    grid_.insert(text, box);
}

void grid_index_collision_detector::clear()
{
    grid_.clear();
}

box2d<double> const& grid_index_collision_detector::extent() const
{
    return extent_;
}

grid_index_collision_detector::grid_index_type::elements_iterator
grid_index_collision_detector::begin() const
{
    return grid_.begin();
}

grid_index_collision_detector::grid_index_type::elements_iterator
grid_index_collision_detector::end() const
{
    return grid_.end();
}

} // namespace mapnik
