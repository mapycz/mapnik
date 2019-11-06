/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2015 Artem Pavlenko
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/

#ifndef MAPNIK_LABEL_COLLISION_DETECTOR_HPP
#define MAPNIK_LABEL_COLLISION_DETECTOR_HPP

// mapnik
#include <mapnik/quad_tree.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/geometry_adapters.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore.hpp>
#include <unicode/unistr.h>
#pragma GCC diagnostic pop

// stl
#include <vector>

#include <boost/geometry/index/rtree.hpp>

namespace mapnik
{
//this needs to be tree structure
//as a proof of a concept _only_ we use sequential scan

struct label_collision_detector
{
    using label_placements = std::vector<box2d<double> >;

    bool has_placement(box2d<double> const& box)
    {
        for (auto const& label : labels_)
        {
            if (label.intersects(box)) return false;
        }
        labels_.push_back(box);
        return true;
    }
    void clear()
    {
        labels_.clear();
    }

private:

    label_placements labels_;
};

// quad_tree based label collision detector
class label_collision_detector2 : util::noncopyable
{
    using tree_t = quad_tree<box2d<double> >;
    tree_t tree_;
public:

    explicit label_collision_detector2(box2d<double> const& extent)
        : tree_(extent) {}

    bool has_placement(box2d<double> const& box)
    {
        tree_t::query_iterator itr = tree_.query_in_box(box);
        tree_t::query_iterator end = tree_.query_end();
        for ( ;itr != end; ++itr)
        {
            if (itr->get().intersects(box)) return false;
        }
        tree_.insert(box,box);
        return true;
    }

    void clear()
    {
        tree_.clear();
    }

};

// quad_tree based label collision detector with seperate check/insert
class label_collision_detector3 : util::noncopyable
{
    using tree_t = quad_tree< box2d<double> >;
    tree_t tree_;
public:

    explicit label_collision_detector3(box2d<double> const& extent)
        : tree_(extent) {}

    bool has_placement(box2d<double> const& box)
    {
        tree_t::query_iterator itr = tree_.query_in_box(box);
        tree_t::query_iterator end = tree_.query_end();

        for ( ;itr != end; ++itr)
        {
            if (itr->get().intersects(box)) return false;
        }
        return true;
    }

    void insert(box2d<double> const& box)
    {
        tree_.insert(box, box);
    }

    void clear()
    {
        tree_.clear();
    }
};


//quad tree based label collision detector so labels dont appear within a given distance
class label_collision_detector4 : util::noncopyable
{
public:
    struct label
    {
        label(box2d<double> const& b) : box(b), text() {}
        label(box2d<double> const& b, mapnik::value_unicode_string const& t) : box(b), text(t) {}

        box2d<double> box;
        mapnik::value_unicode_string text;
    };

private:
    using tree_t = quad_tree< label >;
    tree_t tree_;

public:
    using query_iterator = tree_t::query_iterator;

    explicit label_collision_detector4(box2d<double> const& _extent)
        : tree_(_extent)
#ifdef MAPNIK_STATS_RENDER
          , query_count_(0)
#endif
    {
    }

    bool has_placement(box2d<double> const& box)
    {
#ifdef MAPNIK_STATS_RENDER
        ++query_count_;
#endif

        tree_t::query_iterator tree_itr = tree_.query_in_box(box);
        tree_t::query_iterator tree_end = tree_.query_end();

        for ( ;tree_itr != tree_end; ++tree_itr)
        {
            if (tree_itr->get().box.intersects(box)) return false;
        }

        return true;
    }

    bool has_placement(box2d<double> const& box, double margin)
    {
#ifdef MAPNIK_STATS_RENDER
        ++query_count_;
#endif

        box2d<double> const& margin_box = (margin > 0
                                               ? box2d<double>(box.minx() - margin, box.miny() - margin,
                                                               box.maxx() + margin, box.maxy() + margin)
                                               : box);

        tree_t::query_iterator tree_itr = tree_.query_in_box(margin_box);
        tree_t::query_iterator tree_end = tree_.query_end();

        for (;tree_itr != tree_end; ++tree_itr)
        {
            if (tree_itr->get().box.intersects(margin_box))
            {
                return false;
            }
        }
        return true;
    }

    bool has_placement(box2d<double> const& box, double margin, mapnik::value_unicode_string const& text, double repeat_distance)
    {
#ifdef MAPNIK_STATS_RENDER
        ++query_count_;
#endif

        // Don't bother with any of the repeat checking unless the repeat distance is greater than the margin
        if (repeat_distance <= margin) {
            return has_placement(box, margin);
        }

        box2d<double> repeat_box(box.minx() - repeat_distance, box.miny() - repeat_distance,
                                 box.maxx() + repeat_distance, box.maxy() + repeat_distance);

        box2d<double> const& margin_box = (margin > 0
                                               ? box2d<double>(box.minx() - margin, box.miny() - margin,
                                                               box.maxx() + margin, box.maxy() + margin)
                                               : box);

        tree_t::query_iterator tree_itr = tree_.query_in_box(repeat_box);
        tree_t::query_iterator tree_end = tree_.query_end();

        for ( ;tree_itr != tree_end; ++tree_itr)
        {
            if (tree_itr->get().box.intersects(margin_box) || (text == tree_itr->get().text && tree_itr->get().box.intersects(repeat_box)))
            {
                return false;
            }
        }

        return true;
    }

    void insert(box2d<double> const& box)
    {
        if (tree_.extent().intersects(box))
        {
            tree_.insert(label(box), box);
        }
    }

    void insert(box2d<double> const& box, mapnik::value_unicode_string const& text)
    {
        if (tree_.extent().intersects(box))
        {
            tree_.insert(label(box, text), box);
        }
    }

    void clear()
    {
        tree_.clear();
    }

    box2d<double> const& extent() const
    {
        return tree_.extent();
    }

    query_iterator begin() { return tree_.query_in_box(extent()); }
    query_iterator end() { return tree_.query_end(); }
#ifdef MAPNIK_STATS_RENDER
public:
    unsigned long query_count_;

    int count_items() const
    {
        return tree_.count_items();
    }
#endif
};

class label_collision_detector_boost : util::noncopyable
{
    using balancing = boost::geometry::index::quadratic<16>; // TODO: tune this up
    using value_type = std::pair<box2d<double>, value_unicode_string>;
    using tree_type = boost::geometry::index::rtree<value_type, balancing>;

    tree_type tree_;
    box2d<double> extent_;

public:
    explicit label_collision_detector_boost(box2d<double> const& extent)
        : extent_(extent),
          tree_(tree_type::parameters_type(),
                tree_type::indexable_getter(),
                tree_type::value_equal(),
                tree_type::allocator_type())
    {
    }

    bool has_placement(box2d<double> const& box)
    {
        return tree_.qbegin(boost::geometry::index::intersects(box)) == tree_.qend();
    }

    bool has_placement(box2d<double> const& box, double margin)
    {
        box2d<double> const& margin_box = (margin > 0
                                               ? box2d<double>(box.minx() - margin, box.miny() - margin,
                                                               box.maxx() + margin, box.maxy() + margin)
                                               : box);
        return has_placement(margin_box);
    }

    struct text_equality
    {
        value_unicode_string const& text;

        bool operator()(value_type const& v) const
        {
            return v.second == text;
        }
    };

    bool has_placement(box2d<double> const& box, double margin, value_unicode_string const& text, double repeat_distance)
    {
        // Don't bother with any of the repeat checking unless the repeat distance is greater than the margin
        if (repeat_distance <= margin)
        {
            return has_placement(box, margin);
        }

        if (!has_placement(box, margin))
        {
            return false;
        }

        box2d<double> repeat_box(box.minx() - repeat_distance, box.miny() - repeat_distance,
                                 box.maxx() + repeat_distance, box.maxy() + repeat_distance);

        return tree_.qbegin(boost::geometry::index::intersects(repeat_box) &&
                            boost::geometry::index::satisfies(text_equality{text})) == tree_.qend();
    }

    void insert(box2d<double> const& box)
    {
        if (extent_.intersects(box))
        {
            tree_.insert(std::make_pair(box, value_unicode_string()));
        }
    }

    void insert(box2d<double> const& box, mapnik::value_unicode_string const& text)
    {
        if (extent_.intersects(box))
        {
            tree_.insert(std::make_pair(box, text));
        }
    }

    void clear()
    {
        tree_.clear();
    }

    box2d<double> const& extent() const
    {
        return extent_;
    }

    tree_type::const_iterator begin() { return tree_.begin(); }
    tree_type::const_iterator end() { return tree_.end(); }

#ifdef MAPNIK_STATS_RENDER
public:
    unsigned long query_count_;

    int count_items() const
    {
        return tree_.size();
    }
#endif
};

}

#endif // MAPNIK_LABEL_COLLISION_DETECTOR_HPP
