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

#ifndef MAPNIK_CAIRO_RENDER_PATTERN_HPP
#define MAPNIK_CAIRO_RENDER_PATTERN_HPP

#include <mapnik/image.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/marker_helpers.hpp>
#include <mapnik/cairo/cairo_context.hpp>
#include <mapnik/cairo/cairo_render_vector.hpp>

namespace mapnik {

// TODO: Share code with common_pattern_process_visitor in
// renderer_common
// TODO: Move to cpp file
template <typename Symbolizer>
struct cairo_common_pattern_process_visitor
{
    using image_type = image_rgba8;

    cairo_common_pattern_process_visitor(
        renderer_common const & common,
        Symbolizer const & sym,
        feature_impl const & feature)
        : cairo_common_pattern_process_visitor(common, sym, feature,
            get_optional<value_double, keys::spacing>(
                sym, feature, common.vars_),
            get_optional<value_double, keys::spacing_x>(
                sym, feature, common.vars_),
            get_optional<value_double, keys::spacing_y>(
                sym, feature, common.vars_))
    {
    }

    cairo_surface_ptr operator() (marker_null const &) const
    {
        throw std::runtime_error("This should not have been reached.");
    }

    template <typename Marker>
    cairo_surface_ptr operator() (Marker const & marker) const
    {
        box2d<double> bbox(marker.bounding_box());
        agg::trans_affine tr(transform(bbox));

        if (lacing_ == PATTERN_LACING_MODE_ALTERNATING_GRID)
        {
            return render_pattern_alternating(marker, bbox, tr);
        }
        else
        {
            return render_pattern(marker, bbox, tr);
        }
    }

private:
    cairo_common_pattern_process_visitor(
        renderer_common const & common,
        Symbolizer const & sym,
        feature_impl const & feature,
        boost::optional<value_double> spacing,
        boost::optional<value_double> spacing_x,
        boost::optional<value_double> spacing_y)
        : common_(common),
          sym_(sym),
          feature_(feature),
          opacity_(get<value_double, keys::opacity>(
            sym, feature, common_.vars_)),
          spacing_x_(common.scale_factor_ *
              (spacing_x ? *spacing_x : (spacing ? *spacing : 0))),
          spacing_y_(common.scale_factor_ *
              (spacing_y ? *spacing_y : (spacing ? *spacing : 0))),
          lacing_(get<pattern_lacing_mode_enum>(sym_, keys::lacing))
    {
    }

    agg::trans_affine transform(box2d<double> & bbox) const
    {
        agg::trans_affine tr = agg::trans_affine_scaling(
            common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(
            sym_, keys::image_transform);
        if (image_transform)
        {
            evaluate_transform(tr, feature_, common_.vars_,
                *image_transform, common_.scale_factor_);
        }
        bbox *= tr;
        coord<double, 2> c = bbox.center();
        agg::trans_affine mtx = agg::trans_affine_translation(
            0.5 * bbox.width() - c.x,
            0.5 * bbox.height() - c.y);
        return tr * mtx;
    }

    cairo_surface_ptr render_pattern(
        marker_svg const & marker,
        box2d<double> const & bbox,
        agg::trans_affine tr) const
    {
        double width = std::max(1.0,
            std::round(bbox.width() + spacing_x_));
        double height = std::max(1.0,
            std::round(bbox.height() + spacing_y_));
        cairo_rectangle_t extent { 0, 0, width, height };
        cairo_surface_ptr surface(
            cairo_recording_surface_create(
                CAIRO_CONTENT_COLOR_ALPHA, &extent),
            cairo_surface_closer());

        cairo_ptr cairo = create_context(surface);
        cairo_context context(cairo);

        svg_storage_type & svg = *marker.get_data();
        svg_attribute_type const & svg_attributes = svg.attributes();
        svg_attribute_type custom_attributes;
        bool use_custom_attributes = push_explicit_style(
            svg_attributes, custom_attributes, sym_,
            feature_, common_.vars_);
        svg_attribute_type const & used_attributes =
            use_custom_attributes ?  custom_attributes : svg_attributes;

        svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(
            svg.source());
        svg::svg_path_adapter svg_path(stl_storage);

        tr.translate(spacing_x_ / 2.0, spacing_y_ / 2.0);

        render_vector_marker(context, svg_path, used_attributes,
            bbox, tr, opacity_);

        return surface;
    }

    cairo_surface_ptr render_pattern(
        marker_rgba8 const& marker,
        box2d<double> const & bbox,
        agg::trans_affine tr) const
    {
        cairo_rectangle_t extent { 0, 0,
            bbox.width() + spacing_x_,
            bbox.height() + spacing_y_ };
        cairo_surface_ptr surface(
            cairo_recording_surface_create(
                CAIRO_CONTENT_COLOR_ALPHA, &extent),
            cairo_surface_closer());

        tr.translate(spacing_x_ / 2.0, spacing_y_ / 2.0);

        cairo_ptr cairo = create_context(surface);
        cairo_context context(cairo);

        context.add_image(tr, marker.get_data(), opacity_);

        return surface;
    }

    cairo_surface_ptr render_pattern_alternating(
        marker_svg const & marker,
        box2d<double> const & bbox,
        agg::trans_affine tr) const
    {
        double width = std::max(1.0,
            std::round(bbox.width() + spacing_x_));
        double height = std::max(1.0,
            std::round((bbox.height() + spacing_y_) * 2.0));
        cairo_rectangle_t extent { 0, 0, width, height };
        cairo_surface_ptr surface(
            cairo_recording_surface_create(
                CAIRO_CONTENT_COLOR_ALPHA, &extent),
            cairo_surface_closer());

        cairo_ptr cairo = create_context(surface);
        cairo_context context(cairo);

        svg_storage_type & svg = *marker.get_data();
        svg_attribute_type const & svg_attributes = svg.attributes();
        svg_attribute_type custom_attributes;
        bool use_custom_attributes = push_explicit_style(
            svg_attributes, custom_attributes, sym_,
            feature_, common_.vars_);
        svg_attribute_type const & used_attributes =
            use_custom_attributes ?  custom_attributes : svg_attributes;

        svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(
            svg.source());
        svg::svg_path_adapter svg_path(stl_storage);

        tr.translate(spacing_x_ / 2.0, spacing_y_ / 2.0);

        render_vector_marker(context, svg_path, used_attributes,
            bbox, tr, opacity_);

        tr.translate(std::round(-(bbox.width() / 2.0 + spacing_x_ / 2.0)),
            bbox.height() + spacing_y_);

        render_vector_marker(context, svg_path, used_attributes,
            bbox, tr, opacity_);

        tr.translate(std::round(bbox.width() + spacing_x_), 0);

        render_vector_marker(context, svg_path, used_attributes,
            bbox, tr, opacity_);

        return surface;
    }

    cairo_surface_ptr render_pattern_alternating(
        marker_rgba8 const& marker,
        box2d<double> const & bbox,
        agg::trans_affine tr) const
    {
        cairo_rectangle_t extent { 0, 0,
            bbox.width() + spacing_x_,
            (bbox.height() + spacing_y_) * 2.0 };
        cairo_surface_ptr surface(
            cairo_recording_surface_create(
                CAIRO_CONTENT_COLOR_ALPHA, &extent),
            cairo_surface_closer());

        cairo_ptr cairo = create_context(surface);
        cairo_context context(cairo);

        tr.translate(spacing_x_ / 2.0, spacing_y_ / 2.0);

        context.add_image(tr, marker.get_data(), opacity_);

        tr.translate(std::round(-(bbox.width() / 2.0 + spacing_x_ / 2.0)),
            bbox.height() + spacing_y_);

        context.add_image(tr, marker.get_data(), opacity_);

        tr.translate(std::round(bbox.width() + spacing_x_), 0);

        context.add_image(tr, marker.get_data(), opacity_);

        return surface;
    }

    renderer_common const & common_;
    Symbolizer const & sym_;
    feature_impl const & feature_;
    const value_double opacity_;
    const value_double spacing_x_;
    const value_double spacing_y_;
    const pattern_lacing_mode_enum lacing_;
};

} // namespace mapnik

#endif // MAPNIK_CAIRO_RENDER_PATTERN_HPP
