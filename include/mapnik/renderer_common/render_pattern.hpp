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

#ifndef MAPNIK_RENDER_PATTERN_HPP
#define MAPNIK_RENDER_PATTERN_HPP

#include <mapnik/image.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/util/const_rendering_buffer.hpp>
#include <mapnik/marker_helpers.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_pixfmt_gray.h"
#include "agg_color_rgba.h"
#include "agg_color_gray.h"
#include "agg_scanline_u.h"
#include "agg_image_accessors.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#pragma GCC diagnostic pop

namespace mapnik {

template <typename Symbolizer, typename Rasterizer>
struct common_pattern_process_visitor
{
    using image_type = image_rgba8;

    common_pattern_process_visitor(
        Rasterizer & ras,
        renderer_common const & common,
        Symbolizer const & sym,
        feature_impl const & feature)
        : common_pattern_process_visitor(ras, common, sym, feature,
            get_optional<value_double, keys::spacing>(sym, feature, common.vars_),
            get_optional<value_double, keys::spacing_x>(sym, feature, common.vars_),
            get_optional<value_double, keys::spacing_y>(sym, feature, common.vars_))
    {
    }

    image_type operator() (marker_null const &) const
    {
        throw std::runtime_error("This should not have been reached.");
    }

    template <typename Marker>
    image_type operator() (Marker const & marker) const
    {
        box2d<double> bbox(marker.bounding_box());
        agg::trans_affine tr(transform(bbox));

        if (bbox.width() < 1.0 || bbox.height() < 1.0)
        {
            MAPNIK_LOG_ERROR(common_pattern_process_visitor) << "Pattern image smaller than one pixel";
        }

        if (lacing_ == PATTERN_LACING_MODE_ALTERNATING_GRID)
        {
            return render_pattern_alternating(ras_, marker, bbox, tr);
        }
        return render_pattern(ras_, marker, bbox, tr);
    }

private:
    common_pattern_process_visitor(
        Rasterizer & ras,
        renderer_common const & common,
        Symbolizer const & sym,
        feature_impl const & feature,
        boost::optional<value_double> spacing,
        boost::optional<value_double> spacing_x,
        boost::optional<value_double> spacing_y)
        : ras_(ras),
          common_(common),
          sym_(sym),
          feature_(feature),
          spacing_x_(spacing_x ? *spacing_x : (spacing ? *spacing : 0)),
          spacing_y_(spacing_y ? *spacing_y : (spacing ? *spacing : 0)),
          lacing_(get<pattern_lacing_mode_enum>(sym_, keys::lacing))
    {
    }

    agg::trans_affine transform(box2d<double> & bbox) const
    {
        agg::trans_affine tr = agg::trans_affine_scaling(common_.scale_factor_);
        auto image_transform = get_optional<transform_type>(sym_, keys::image_transform);
        if (image_transform) evaluate_transform(tr, feature_, common_.vars_, *image_transform, common_.scale_factor_);
        bbox *= tr;
        coord<double, 2> c = bbox.center();
        agg::trans_affine mtx = agg::trans_affine_translation(
            0.5 * bbox.width() - c.x,
            0.5 * bbox.height() - c.y);
        return tr * mtx;
    }

    image_rgba8 render_pattern(rasterizer & ras,
                               marker_svg const & marker,
                               box2d<double> const & bbox,
                               agg::trans_affine tr) const
    {
        using pixfmt = agg::pixfmt_rgba32_pre;
        using renderer_base = agg::renderer_base<pixfmt>;
        using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;
        agg::scanline_u8 sl;

        image_rgba8 image(bbox.width() + spacing_x_, bbox.height() + spacing_y_);
        agg::rendering_buffer buf(image.bytes(), image.width(), image.height(), image.row_size());
        pixfmt pixf(buf);
        renderer_base renb(pixf);

        svg_storage_type & svg = *marker.get_data();
        svg_attribute_type const & svg_attributes = svg.attributes();
        svg_attribute_type custom_attributes;
        bool use_custom_attributes = push_explicit_style(
            svg_attributes, custom_attributes, sym_, feature_, common_.vars_);
        svg_attribute_type const & used_attributes = use_custom_attributes ?
            custom_attributes : svg_attributes;

        svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(svg.source());
        svg::svg_path_adapter svg_path(stl_storage);
        using renderer_type = svg::svg_renderer_agg<svg::svg_path_adapter,
            agg::pod_bvector<svg::path_attributes>, renderer_solid, pixfmt>;
        renderer_type svg_renderer(svg_path, used_attributes);

        tr.translate(spacing_x_ / 2.0, spacing_y_ / 2.0);
        svg_renderer.render(ras, sl, renb, tr, 1.0, bbox);
        return image;
    }

    image_rgba8 render_pattern(rasterizer & ras,
                               marker_rgba8 const& marker,
                               box2d<double> const & bbox,
                               agg::trans_affine tr) const
    {
        using pixfmt = agg::pixfmt_rgba32_pre;
        using renderer_base = agg::renderer_base<pixfmt>;
        using renderer = agg::renderer_scanline_aa_solid<renderer_base>;

        image_rgba8 image(bbox.width() + spacing_x_, bbox.height() + spacing_y_);
        agg::rendering_buffer buf_out(image.bytes(), image.width(), image.height(), image.row_size());
        pixfmt pixf_out(buf_out);
        renderer_base rb(pixf_out);
        renderer r(rb);

        using const_rendering_buffer = util::rendering_buffer<image_rgba8>;
        using pixfmt_in = agg::pixfmt_alpha_blend_rgba<agg::blender_rgba32_pre, const_rendering_buffer, agg::pixel32_type>;

        image_rgba8 const& src = marker.get_data();
        const_rendering_buffer buf_in(src);
        pixfmt_in pixf(buf_in);

        tr.translate(spacing_x_ / 2.0, spacing_y_ / 2.0);
        tr.invert();

        using interpolator_type = agg::span_interpolator_linear<>;
        interpolator_type interpolator(tr);

        agg::span_allocator<agg::rgba8> sa;

        agg::image_filter_lut filter;
        filter.calculate(agg::image_filter_bilinear(), true);

        using img_accessor_type = agg::image_accessor_clone<pixfmt_in>;
        using span_gen_type = agg::span_image_resample_rgba_affine<img_accessor_type>;
        img_accessor_type ia(pixf);
        span_gen_type sg(ia, interpolator, filter);

        agg::scanline_u8 sl;
        ras.move_to_d(0, 0);
        ras.line_to_d(image.width(), 0);
        ras.line_to_d(image.width(), image.height());
        ras.line_to_d(0, image.height());

        agg::render_scanlines_aa(ras, sl, rb, sa, sg);

        return image;
    }

    image_rgba8 render_pattern_alternating(rasterizer & ras,
                               marker_svg const & marker,
                               box2d<double> const & bbox,
                               agg::trans_affine tr) const
    {
        using pixfmt = agg::pixfmt_rgba32_pre;
        using renderer_base = agg::renderer_base<pixfmt>;
        using renderer_solid = agg::renderer_scanline_aa_solid<renderer_base>;
        agg::scanline_u8 sl;

        image_rgba8 image(
            std::round(bbox.width() + spacing_x_),
            std::round((bbox.height() + spacing_y_) * 2.0));
        agg::rendering_buffer buf(image.bytes(), image.width(), image.height(), image.row_size());
        pixfmt pixf(buf);
        renderer_base renb(pixf);

        svg_storage_type & svg = *marker.get_data();
        svg_attribute_type const & svg_attributes = svg.attributes();
        svg_attribute_type custom_attributes;
        bool use_custom_attributes = push_explicit_style(
            svg_attributes, custom_attributes, sym_, feature_, common_.vars_);
        svg_attribute_type const & used_attributes = use_custom_attributes ?
            custom_attributes : svg_attributes;

        svg::vertex_stl_adapter<svg::svg_path_storage> stl_storage(svg.source());
        svg::svg_path_adapter svg_path(stl_storage);
        using renderer_type = svg::svg_renderer_agg<svg::svg_path_adapter,
            agg::pod_bvector<svg::path_attributes>, renderer_solid, pixfmt>;
        renderer_type svg_renderer(svg_path, used_attributes);

        tr.translate(spacing_x_ / 2.0, spacing_y_ / 2.0);
        svg_renderer.render(ras, sl, renb, tr, 1.0, bbox);
        tr.translate(std::round(-(bbox.width() / 2.0 + spacing_x_ / 2.0)),
            bbox.height() + spacing_y_);
        svg_renderer.render(ras, sl, renb, tr, 1.0, bbox);
        tr.translate(std::round(bbox.width() + spacing_x_), 0);
        svg_renderer.render(ras, sl, renb, tr, 1.0, bbox);
        return image;
    }

    image_rgba8 render_pattern_alternating(rasterizer & ras,
                               marker_rgba8 const& marker,
                               box2d<double> const & bbox,
                               agg::trans_affine tr) const
    {
        using pixfmt = agg::pixfmt_rgba32_pre;
        using renderer_base = agg::renderer_base<pixfmt>;
        using renderer = agg::renderer_scanline_aa_solid<renderer_base>;

        image_rgba8 image(
            std::round(bbox.width() + spacing_x_),
            std::round((bbox.height() + spacing_y_) * 2.0));
        agg::rendering_buffer buf_out(image.bytes(), image.width(), image.height(), image.row_size());
        pixfmt pixf_out(buf_out);
        renderer_base rb(pixf_out);
        renderer r(rb);

        using const_rendering_buffer = util::rendering_buffer<image_rgba8>;
        using pixfmt_in = agg::pixfmt_alpha_blend_rgba<agg::blender_rgba32_pre, const_rendering_buffer, agg::pixel32_type>;

        image_rgba8 const& src = marker.get_data();
        const_rendering_buffer buf_in(src);
        pixfmt_in pixf(buf_in);

        tr.translate(spacing_x_ / 2.0, spacing_y_ / 2.0);
        tr.invert();

        using interpolator_type = agg::span_interpolator_linear<>;
        interpolator_type interpolator(tr);

        agg::span_allocator<agg::rgba8> sa;

        agg::image_filter_lut filter;
        filter.calculate(agg::image_filter_bilinear(), true);

        using img_accessor_type = agg::image_accessor_clone<pixfmt_in>;
        using span_gen_type = agg::span_image_resample_rgba_affine<img_accessor_type>;
        img_accessor_type ia(pixf);
        span_gen_type sg(ia, interpolator, filter);

        agg::scanline_u8 sl;
        ras.move_to_d(0, 0);
        ras.line_to_d(image.width(), 0);
        ras.line_to_d(image.width(), image.height());
        ras.line_to_d(0, image.height());

        agg::render_scanlines_aa(ras, sl, rb, sa, sg);

        tr.invert();
        tr.translate(std::round(-(bbox.width() / 2.0 + spacing_x_ / 2.0)),
            bbox.height() + spacing_y_);
        tr.invert();
        agg::render_scanlines_aa(ras, sl, rb, sa, sg);

        tr.invert();
        tr.translate(std::round(bbox.width() + spacing_x_), 0);
        tr.invert();
        agg::render_scanlines_aa(ras, sl, rb, sa, sg);

        return image;
    }

    Rasterizer & ras_;
    renderer_common const & common_;
    Symbolizer const & sym_;
    feature_impl const & feature_;
    const value_double spacing_x_;
    const value_double spacing_y_;
    const pattern_lacing_mode_enum lacing_;
};

} // namespace mapnik

#endif // MAPNIK_RENDER_PATTERN_HPP
