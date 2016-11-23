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
#include <mapnik/label_placement.hpp>
#include <mapnik/marker.hpp>

namespace mapnik {

class feature_impl;
class proj_transform;
class view_transform;
struct symbolizer_base;

/*
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
    proj_transform const& prj_trans_;
    view_transform const& t_;
    box2d<double> dims_;
    box2d<double> const& query_extent_;

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
*/
// Helper object that does all the TextSymbolizer placement finding
// work except actually rendering the object.

class text_symbolizer_helper //: public base_symbolizer_helper
{
public:
    template <typename FaceManagerT, typename DetectorT>
    static placements_list get(symbolizer_base const& sym,
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
                           agg::trans_affine const affine_trans,
                           symbol_cache const& sc)
    {
        box2d<double> dims(0, 0, width, height);
        text_placement_info_ptr info_ptr = mapnik::get<text_placements_ptr>(
            sym, keys::text_placements_)->get_placement_info(scale_factor,
                feature, vars, sc);
        placement_finder finder(feature, vars, detector,
            dims, *info_ptr, font_manager, scale_factor);
        evaluated_text_properties_ptr text_props(evaluate_text_properties(
            info_ptr->properties, feature, vars));

        label_placement_enum placement_type = text_props->label_placement;

        label_placement::placement_params params {
            detector, font_manager, finder, prj_trans, t, affine_trans, sym,
            feature, vars, box2d<double>(0, 0, width, height), query_extent,
            scale_factor, sc };

        return label_placement::finder::get(placement_type, params);
    }

    // Return all placements.
    placements_list get() const;
protected:
    //void init_converters();
    //void initialize_points() const;
    //bool next_point_placement() const;
    //template <typename T>
    //bool next_line_placement(T const & adapter) const;

    //mutable placement_finder finder_;

    //mutable vertex_converter_type converter_;

    ////
    //mutable label_placement::placement_params params_;
};

class shield_symbolizer_helper
{
public:
    template <typename FaceManagerT, typename DetectorT>
    static placements_list get(symbolizer_base const& sym,
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
                           agg::trans_affine const affine_trans,
                           symbol_cache const& sc)
    {
        box2d<double> dims(0, 0, width, height);
        text_placement_info_ptr info_ptr = mapnik::get<text_placements_ptr>(
            sym, keys::text_placements_)->get_placement_info(scale_factor,
                feature, vars, sc);
        placement_finder finder(feature, vars, detector,
            dims, *info_ptr, font_manager, scale_factor);
        evaluated_text_properties_ptr text_props(evaluate_text_properties(
            info_ptr->properties, feature, vars));
        init_marker(finder, sym, feature, vars, scale_factor);

        label_placement_enum placement_type = text_props->label_placement;

        label_placement::placement_params params {
            detector, font_manager, finder, prj_trans, t, affine_trans, sym,
            feature, vars, box2d<double>(0, 0, width, height), query_extent,
            scale_factor, sc };

        return label_placement::finder::get(placement_type, params);
    }

protected:
    static void init_marker(placement_finder & finder,
                            symbolizer_base const& sym,
                            feature_impl const& feature,
                            attributes const& vars,
                            double scale_factor)
    {
        std::string filename = mapnik::get<std::string,keys::file>(sym, feature, vars);
        if (filename.empty()) return;
        std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);
        if (marker->is<marker_null>()) return;
        agg::trans_affine trans;
        auto image_transform = get_optional<transform_type>(sym, keys::image_transform);
        if (image_transform) evaluate_transform(trans, feature, vars, *image_transform, scale_factor);
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
        value_bool unlock_image = mapnik::get<value_bool, keys::unlock_image>(sym, feature, vars);
        value_double shield_dx = mapnik::get<value_double, keys::shield_dx>(sym, feature, vars);
        value_double shield_dy = mapnik::get<value_double, keys::shield_dy>(sym, feature, vars);
        pixel_position marker_displacement;
        marker_displacement.set(shield_dx,shield_dy);
        finder.set_marker(std::make_shared<marker_info>(marker, trans), bbox, unlock_image, marker_displacement);
    }
};

} //namespace mapnik

#endif // SYMBOLIZER_HELPERS_HPP
