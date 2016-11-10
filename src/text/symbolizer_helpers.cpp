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

// mapnik
#include <mapnik/text/symbolizer_helpers.hpp>
#include <mapnik/label_collision_detector.hpp>
#include <mapnik/font_engine_freetype.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/geometry_type.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/geometry_centroid.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/value_types.hpp>
#include <mapnik/text/placement_finder_impl.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/text/placements/dummy.hpp>
#include <mapnik/proj_strategy.hpp>
#include <mapnik/view_strategy.hpp>

namespace mapnik {
    /*
namespace geometry {

struct envelope_impl
{
    template <typename T>
    box2d<double> operator() (T const& ref) const
    {
        return envelope<T>(ref);
    }
};

mapnik::box2d<double> envelope(mapnik::base_symbolizer_helper::geometry_cref const& geom)
{
    return mapnik::util::apply_visitor(envelope_impl(), geom);
}

struct geometry_type_impl
{
    template <typename T>
    auto operator() (T const& ref) const -> decltype(geometry_type<T>(ref))
    {
        return geometry_type<T>(ref);
    }
};

mapnik::geometry::geometry_types geometry_type(mapnik::base_symbolizer_helper::geometry_cref const& geom)
{
    return mapnik::util::apply_visitor(geometry_type_impl(), geom);
}

} // geometry
namespace detail {

struct placement_finder_adapter
{
    placement_finder_adapter(placement_finder & finder)
        : finder_(finder) {}

    template <typename PathT>
    void add_path(PathT & path) const
    {
        status_ = finder_.find_line_placements(path, false);
    }

    bool status() const { return status_; }
    placement_finder & finder_;
    mutable bool status_ = false;
};

struct point_placement_finder_adapter : placement_finder_adapter
{
    using placement_finder_adapter::placement_finder_adapter;

    template <typename PathT>
    void add_path(PathT & path) const
    {
        status_ = finder_.find_line_placements(path, true);
    }
};

template <typename T>
struct split_multi_geometries
{
    using container_type = T;
    split_multi_geometries(container_type & cont)
        : cont_(cont) { }

    void operator() (geometry::geometry_empty const&) const {}
    void operator() (geometry::multi_point<double> const& multi_pt) const
    {
        for ( auto const& pt : multi_pt )
        {
            cont_.push_back(base_symbolizer_helper::geometry_cref(std::cref(pt)));
        }
    }
    void operator() (geometry::line_string<double> const& line) const
    {
        cont_.push_back(base_symbolizer_helper::geometry_cref(std::cref(line)));
    }

    void operator() (geometry::multi_line_string<double> const& multi_line) const
    {
        for ( auto const& line : multi_line )
        {
            (*this)(line);
        }
    }

    void operator() (geometry::polygon<double> const& poly) const
    {
        cont_.push_back(base_symbolizer_helper::geometry_cref(std::cref(poly)));
    }

    void operator() (geometry::multi_polygon<double> const& multi_poly) const
    {
        for ( auto const& poly : multi_poly )
        {
            (*this)(poly);
        }
    }

    void operator() (geometry::geometry_collection<double> const& collection) const
    {
        for ( auto const& geom : collection)
        {
            util::apply_visitor(*this, geom);
        }
    }

    template <typename Geometry>
    void operator() (Geometry const& geom) const
    {
        cont_.push_back(base_symbolizer_helper::geometry_cref(std::cref(geom)));
    }

    container_type & cont_;
};

} // ns detail

base_symbolizer_helper::base_symbolizer_helper(
        symbolizer_base const& sym,
        feature_impl const& feature,
        attributes const& vars,
        proj_transform const& prj_trans,
        unsigned width, unsigned height, double scale_factor,
        view_transform const& t,
        box2d<double> const& query_extent,
        symbol_cache const& sc)
    : sym_(sym),
      feature_(feature),
      vars_(vars),
      prj_trans_(prj_trans),
      t_(t),
      dims_(0, 0, width, height),
      query_extent_(query_extent),
      scale_factor_(scale_factor),
      info_ptr_(mapnik::get<text_placements_ptr>(sym_, keys::text_placements_)->get_placement_info(scale_factor,feature_,vars_, sc)),
      text_props_(evaluate_text_properties(info_ptr_->properties,feature_,vars_))
{
    initialize_geometries();
    if (!geometries_to_process_.size()) return; // FIXME - bad practise
    initialize_points();
}

struct largest_bbox_first
{
    bool operator() (geometry::geometry<double> const* g0, geometry::geometry<double> const* g1) const
    {
        box2d<double> b0 = geometry::envelope(*g0);
        box2d<double> b1 = geometry::envelope(*g1);
        return b0.width() * b0.height() > b1.width() * b1.height();
    }
    bool operator() (base_symbolizer_helper::geometry_cref const& g0,
                     base_symbolizer_helper::geometry_cref const& g1) const
    {
        // TODO - this has got to be expensive! Can we cache bbox's if there are repeated calls to same geom?
        box2d<double> b0 = geometry::envelope(g0);
        box2d<double> b1 = geometry::envelope(g1);
        return b0.width() * b0.height() > b1.width() * b1.height();
    }
};

void base_symbolizer_helper::initialize_geometries() const
{
    auto const& geom = feature_.get_geometry();
    util::apply_visitor(detail::split_multi_geometries<geometry_container_type>(geometries_to_process_), geom);
    if (!geometries_to_process_.empty())
    {
        auto type = geometry::geometry_type(geom);
        if (type == geometry::geometry_types::Polygon ||
            type == geometry::geometry_types::MultiPolygon)
        {
            bool largest_box_only = text_props_->largest_bbox_only;
            if (largest_box_only)
            {
                geometries_to_process_.sort(largest_bbox_first());
                geo_itr_ = geometries_to_process_.begin();
                geometries_to_process_.erase(++geo_itr_, geometries_to_process_.end());
            }
        }
        geo_itr_ = geometries_to_process_.begin();
    }
}
*/

template <typename FaceManagerT, typename DetectorT>
text_symbolizer_helper::text_symbolizer_helper(
        symbolizer_base const& sym,
        feature_impl const& feature,
        attributes const& vars,
        proj_transform const& prj_trans,
        unsigned width, unsigned height, double scale_factor,
        view_transform const& t, FaceManagerT & font_manager,
        DetectorT &detector, box2d<double> const& query_extent,
        agg::trans_affine const& affine_trans,
        symbol_cache const& sc)
    : params_({ detector, font_manager, prj_trans, t, affine_trans, sym, feature, vars,
        box2d<double>(0, 0, width, height), query_extent, scale_factor, sc })

    //: base_symbolizer_helper(sym, feature, vars, prj_trans, width, height, scale_factor, t, query_extent, sc),
      //finder_(feature, vars, detector, dims_, *info_ptr_, font_manager, scale_factor),
    //converter_(query_extent_, sym_, t, prj_trans, affine_trans, feature, vars, scale_factor)
{
    // setup vertex converter
    /*value_bool clip = mapnik::get<value_bool, keys::clip>(sym_, feature_, vars_);
    value_double simplify_tolerance = mapnik::get<value_double, keys::simplify_tolerance>(sym_, feature_, vars_);
    value_double smooth = mapnik::get<value_double, keys::smooth>(sym_, feature_, vars_);
    value_double extend = mapnik::get<value_double, keys::extend>(sym_, feature_, vars_);

    if (clip)
    {
        label_placement_enum how_placed = text_props_->label_placement;
        if (how_placed == GRID_PLACEMENT)
        {
            converter_.template set<clip_poly_tag>();
        }
        else
        {
            converter_.template set<clip_line_tag>();
        }
    }

    converter_.template set<transform_tag>(); //always transform
    converter_.template set<affine_transform_tag>();
    if (extend > 0.0) converter_.template set<extend_tag>();
    if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>(); // optional simplify converter
    if (smooth > 0.0) converter_.template set<smooth_tag>(); // optional smooth converter*/
}

placements_list text_symbolizer_helper::get() const
{
    text_placement_info_ptr info_ptr = mapnik::get<text_placements_ptr>(
        params_.symbolizer, keys::text_placements_)->get_placement_info(params_.scale_factor, params_.feature, params_.vars, params_.symbol_cache);
    evaluated_text_properties_ptr text_props(evaluate_text_properties(
        info_ptr->properties,params_.feature,params_.vars));

    label_placement_enum placement_type = text_props->label_placement;

    return label_placement::finder::get(placement_type, params_);
    /*if (!geometries_to_process_.empty())
    {
        finder_.next_position();

        if (point_placement_)
        {
            while (next_point_placement());
        }
        else
        {
            detail::placement_finder_adapter adapter(finder_);
            while (next_line_placement(adapter));
        }
    }
    return finder_.placements();*/
}
/*
placements_list const& shield_symbolizer_helper::get() const
{
    if (!geometries_to_process_.empty())
    {
        init_marker();
        finder_.next_position();

        if (point_placement_)
        {
            while (next_point_placement());
        }
        else
        {
            detail::point_placement_finder_adapter adapter(finder_);
            while (next_line_placement(adapter));
        }
    }
    return finder_.placements();
}

template <typename Adapter>
class apply_line_placement_visitor
{
public:
    apply_line_placement_visitor(vertex_converter_type & converter,
                                 Adapter const & adapter)
        : converter_(converter), adapter_(adapter)
    {
    }

    bool operator()(geometry::line_string<double> const & geo) const
    {
        geometry::line_string_vertex_adapter<double> va(geo);
        converter_.apply(va, adapter_);
        return adapter_.status();
    }

    bool operator()(geometry::polygon<double> const & geo) const
    {
        geometry::polygon_vertex_adapter<double> va(geo);
        converter_.apply(va, adapter_);
        return adapter_.status();
    }

    template <typename T>
    bool operator()(T const&) const
    {
        return false;
    }

private:
    vertex_converter_type & converter_;
    Adapter const & adapter_;
};

template <typename T>
bool text_symbolizer_helper::next_line_placement(T const & adapter) const
{
    while (!geometries_to_process_.empty())
    {
        if (geo_itr_ == geometries_to_process_.end())
        {
            //Just processed the last geometry. Try next placement.
            if (!finder_.next_position()) return false; //No more placements
            //Start again from begin of list
            geo_itr_ = geometries_to_process_.begin();
            continue; //Reexecute size check
        }

        if (mapnik::util::apply_visitor(apply_line_placement_visitor<T>(converter_, adapter), *geo_itr_))
        {
            //Found a placement
            geo_itr_ = geometries_to_process_.erase(geo_itr_);
            return true;
        }

        // No placement for this geometry. Keep it in geometries_to_process_ for next try.
        ++geo_itr_;
    }
    return false;
}

bool text_symbolizer_helper::next_point_placement() const
{
    while (!points_.empty())
    {
        if (point_itr_ == points_.end())
        {
            //Just processed the last point. Try next placement.
            if (!finder_.next_position()) return false; //No more placements
            //Start again from begin of list
            point_itr_ = points_.begin();
            continue; //Reexecute size check
        }
        if (finder_.find_point_placement(*point_itr_))
        {
            //Found a placement
            point_itr_ = points_.erase(point_itr_);
            return true;
        }
        //No placement for this point. Keep it in points_ for next try.
        ++point_itr_;
    }
    return false;
}

void shield_symbolizer_helper::init_marker() const
{
    std::string filename = mapnik::get<std::string,keys::file>(sym_, feature_, vars_);
    if (filename.empty()) return;
    std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);
    if (marker->is<marker_null>()) return;
    agg::trans_affine trans;
    auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
    if (image_transform) evaluate_transform(trans, feature_, vars_, *image_transform, scale_factor_);
    double width = marker->width();
    double height = marker->height();
    double px0 = - 0.5 * width;
    double py0 = - 0.5 * height;
    double px1 = 0.5 * width;
    double py1 = 0.5 * height;
    double px2 = px1;
    double py2 = py0;
    double px3 = px0;
    double py3 = py1;
    trans.transform(&px0, &py0);
    trans.transform(&px1, &py1);
    trans.transform(&px2, &py2);
    trans.transform(&px3, &py3);
    box2d<double> bbox(px0, py0, px1, py1);
    bbox.expand_to_include(px2, py2);
    bbox.expand_to_include(px3, py3);
    value_bool unlock_image = mapnik::get<value_bool, keys::unlock_image>(sym_, feature_, vars_);
    value_double shield_dx = mapnik::get<value_double, keys::shield_dx>(sym_, feature_, vars_);
    value_double shield_dy = mapnik::get<value_double, keys::shield_dy>(sym_, feature_, vars_);
    pixel_position marker_displacement;
    marker_displacement.set(shield_dx,shield_dy);
    finder_.set_marker(std::make_shared<marker_info>(marker, trans), bbox, unlock_image, marker_displacement);
}

void text_symbolizer_helper::initialize_points() const
{
    label_placement_enum how_placed = text_props_->label_placement;

    if (how_placed == GRID_PLACEMENT)
    {
        for (auto const& geom : geometries_to_process_)
        {
            auto type = geometry::geometry_type(geom);
            if (type == geometry::geometry_types::Polygon)
            {
                detail::grid_placement_finder_adapter<double, std::list<pixel_position>> ga(
                    text_props_->grid_cell_width, text_props_->grid_cell_height, points_);
                auto const& poly = mapnik::util::get<geometry::polygon<double>>(geom);
                geometry::polygon_vertex_adapter<double> va(poly);
                converter_.apply(va, ga);
            }
        }
    }
    point_itr_ = points_.begin();
}*/

template text_symbolizer_helper::text_symbolizer_helper(
    symbolizer_base const& sym,
    feature_impl const& feature,
    attributes const& vars,
    proj_transform const& prj_trans,
    unsigned width,
    unsigned height,
    double scale_factor,
    view_transform const& t,
    face_manager_freetype & font_manager,
    label_collision_detector4 &detector,
    box2d<double> const& query_extent,
    agg::trans_affine const&,
    symbol_cache const& sc);

} //namespace
