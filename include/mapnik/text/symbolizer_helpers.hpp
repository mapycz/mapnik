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
#ifndef SYMBOLIZER_HELPERS_HPP
#define SYMBOLIZER_HELPERS_HPP

// mapnik
#include <mapnik/text/placement_finder.hpp>
#include <mapnik/text/placements/base.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/symbol_cache.hpp>
#include <mapnik/geometry.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/text_properties.hpp>

namespace mapnik {

class feature_impl;
class proj_transform;
class view_transform;
struct symbolizer_base;

using vertex_converter_type = vertex_converter<clip_line_tag, clip_poly_tag, transform_tag, affine_transform_tag, extend_tag, simplify_tag, smooth_tag>;

class base_symbolizer_helper
{
public:

    using point_cref = std::reference_wrapper<geometry::point<double> const>;
    using line_string_cref = std::reference_wrapper<geometry::line_string<double> const>;
    using polygon_cref = std::reference_wrapper<geometry::polygon<double> const>;
    using geometry_cref = util::variant<point_cref, line_string_cref, polygon_cref>;
    // Using list instead of vector, because we delete random elements and need iterators to stay valid.
    using geometry_container_type = std::list<geometry_cref>;
    base_symbolizer_helper(symbolizer_base const& sym,
                           feature_impl const& feature,
                           attributes const& vars,
                           proj_transform const& prj_trans,
                           unsigned width,
                           unsigned height,
                           double scale_factor,
                           view_transform const& t,
                           box2d<double> const& query_extent,
                           symbol_cache const& sc);

protected:
    void initialize_geometries() const;
    void initialize_points() const;

    //Input
    symbolizer_base const& sym_;
    feature_impl const& feature_;
    attributes const& vars_;
    proj_transform const& prj_trans_;
    view_transform const& t_;
    box2d<double> dims_;
    box2d<double> const& query_extent_;
    double scale_factor_;

    //Processing
    // Remaining geometries to be processed.
    mutable geometry_container_type geometries_to_process_;
    // Geometry currently being processed.
    mutable geometry_container_type::iterator geo_itr_;
    // Remaining points to be processed.
    mutable std::list<pixel_position> points_;
    // Point currently being processed.
    mutable std::list<pixel_position>::iterator point_itr_;
    // Use point placement. Otherwise line placement is used.
    mutable bool point_placement_;
    text_placement_info_ptr info_ptr_;
    evaluated_text_properties_ptr text_props_;
};

// Helper object that does all the TextSymbolizer placement finding
// work except actually rendering the object.

class text_symbolizer_helper : public base_symbolizer_helper
{
public:
    template <typename FaceManagerT, typename DetectorT>
    text_symbolizer_helper(symbolizer_base const& sym,
                           feature_impl const& feature,
                           attributes const& vars,
                           proj_transform const& prj_trans,
                           unsigned width,
                           unsigned height,
                           double scale_factor,
                           view_transform const& t,
                           FaceManagerT & font_manager,
                           DetectorT & detector,
                           box2d<double> const& query_extent,
                           agg::trans_affine const&,
                           symbol_cache const& sc);

    // Return all placements.
    placements_list const& get() const;
protected:
    void init_converters();
    void initialize_points() const;
    bool next_point_placement() const;
    template <typename T>
    bool next_line_placement(T const & adapter) const;

    mutable placement_finder finder_;

    mutable vertex_converter_type converter_;
};

class shield_symbolizer_helper : public text_symbolizer_helper
{
public:
    using text_symbolizer_helper::text_symbolizer_helper;

    placements_list const& get() const;

protected:
    void init_marker() const;
};

namespace geometry {
MAPNIK_DECL mapnik::box2d<double> envelope(mapnik::base_symbolizer_helper::geometry_cref const& geom);
}

} //namespace mapnik

#endif // SYMBOLIZER_HELPERS_HPP
