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
#ifndef LINE_LAYOUT_HPP
#define LINE_LAYOUT_HPP

//mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/pixel_position.hpp>
#include <mapnik/text/text_layout.hpp>
#include <mapnik/text/glyph_positions.hpp>
#include <mapnik/text/rotation.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/extend_converter.hpp>
#include <mapnik/vertex_cache.hpp>
#include <mapnik/text/point_layout.hpp>
#include <mapnik/tolerance_iterator.hpp>
#include <mapnik/text/text_line_policy.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_adapters.hpp>

namespace mapnik
{

template <typename Layout>
struct position_accessor
{
    template <typename T>
    static T & get(T & geom)
    {
        return geom;
    }
};

template <>
struct position_accessor<point_layout>
{
    static pixel_position const & get(vertex_cache const & geom)
    {
        return geom.current_position();
    }

    static pixel_position const & get(pixel_position const & geom)
    {
        return geom;
    }
};

template <>
struct position_accessor<shield_layout> : position_accessor<point_layout>
{
};

// ===========================================

template <typename Layout, typename LayoutGenerator, typename Detector>
struct placement_finder_adapter
{
    placement_finder_adapter(Layout & layout,
        LayoutGenerator & layout_generator,
        Detector & detector)
        : layout_(layout),
          layout_generator_(layout_generator),
          detector_(detector)
    {
    }

    template <typename PathT>
    void add_path(PathT & path) const
    {
        status_ = layout_.try_placement(layout_generator_, detector_, path);
    }

    bool status() const { return status_; }
    Layout & layout_;
    LayoutGenerator & layout_generator_;
    Detector & detector_;
    mutable bool status_ = false;
};

template <typename Adapter, typename VC>
class line_placement_visitor
{
public:
    line_placement_visitor(
        VC & converter,
        Adapter const & adapter)
        : converter_(converter),
          adapter_(adapter)
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
    VC & converter_;
    Adapter const & adapter_;
};

template <typename SubLayout>
class text_vertex_converter : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    using vertex_converter_type = vertex_converter<
        clip_line_tag,
        clip_poly_tag,
        transform_tag,
        affine_transform_tag,
        extend_tag,
        simplify_tag,
        smooth_tag>;

    text_vertex_converter(params_type const & params)
    : sublayout_(params),
      converter_(params.query_extent, params.symbolizer,
        params.view_transform, params.proj_transform,
        params.affine_transform, params.feature, params.vars,
        params.scale_factor)
    {
        value_bool clip = params.get<value_bool, keys::clip>();
        value_double simplify_tolerance = params.get<value_double, keys::simplify_tolerance>();
        value_double smooth = params.get<value_double, keys::smooth>();
        value_double extend = params.get<value_double, keys::extend>();

        if (clip) converter_.template set<clip_line_tag>();
        converter_.template set<transform_tag>();
        converter_.template set<affine_transform_tag>();
        if (extend > 0.0) converter_.template set<extend_tag>();
        if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>();
        if (smooth > 0.0) converter_.template set<smooth_tag>();
    }

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom & geom)
    {
        using adapter_type = placement_finder_adapter<SubLayout, LayoutGenerator, Detector>;
        using visitor_type = line_placement_visitor<adapter_type, vertex_converter_type>;
        adapter_type adapter(sublayout_, layout_generator, detector);
        visitor_type visitor(converter_, adapter);
        return util::apply_visitor(visitor, geom);
    }

private:
    SubLayout sublayout_;
    vertex_converter_type converter_;
};

// ===========================================

static const double halign_adjust_extend = 1000;

template <typename SubLayout>
class text_extend_line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    text_extend_line_layout(params_type const & params);

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom & geom);

    inline double get_length(text_layout_generator const & layout_generator) const
    {
        return layout_generator.layouts_->width();
    }

private:
    SubLayout sublayout_;
    params_type const & params_;
};

template <typename SubLayout>
text_extend_line_layout<SubLayout>::text_extend_line_layout(
    params_type const & params)
    : sublayout_(params),
      params_(params)
{
}

template <typename SubLayout>
template <typename LayoutGenerator, typename Detector, typename Geom>
bool text_extend_line_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    Geom & geom)
{
    layout_container & layouts = *layout_generator.layouts_;

    if (!layouts.line_count()) return true;

    horizontal_alignment_e halign = layouts.root_layout().horizontal_alignment();
    if (halign == H_ADJUST)
    {
        extend_converter<Geom> ec(geom, halign_adjust_extend);
        return sublayout_.try_placement(layout_generator, detector, ec);
    }

    return sublayout_.try_placement(layout_generator, detector, geom);
}

template <typename SubLayout>
class line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    line_layout(params_type const & params);

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom & geom);

protected:
    template <typename LayoutGenerator, typename LineLayoutPolicy, typename Detector>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        vertex_cache & path,
        LineLayoutPolicy & policy);

    SubLayout sublayout_;
    params_type const & params_;
};

template <typename SubLayout>
line_layout<SubLayout>::line_layout(params_type const & params)
    : sublayout_(params),
      params_(params)
{
}

template <typename SubLayout>
template <typename LayoutGenerator, typename Detector, typename Geom>
bool line_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    Geom & geom)
{
    vertex_cache path(geom);
    double layout_width = sublayout_.get_length(layout_generator);
    text_line_policy<LayoutGenerator> policy(path, layout_generator, layout_width, params_);
    return try_placement(layout_generator, detector, path, policy);
}

template <typename SubLayout>
template <typename LayoutGenerator, typename LineLayoutPolicy, typename Detector>
bool line_layout<SubLayout>::try_placement(
    LayoutGenerator & layout_generator,
    Detector & detector,
    vertex_cache & path,
    LineLayoutPolicy & policy)
{
    bool success = false;
    while (path.next_subpath())
    {
        if (!policy.check_size())
        {
            continue;
        }

        if (!policy.align())
        {
            continue;
        }

        double spacing = policy.get_spacing();

        do
        {
            tolerance_iterator<exponential_function> tolerance_offset(
                policy.position_tolerance());
            while (tolerance_offset.next())
            {
                vertex_cache::scoped_state state(path);
                if (policy.move(tolerance_offset.get()) &&
                    sublayout_.try_placement(layout_generator, detector,
                        position_accessor<SubLayout>::get(path)))
                {
                    success = true;
                    break;
                }
            }
        } while (path.forward(spacing));
    }
    return success;
}


class single_line_layout : util::noncopyable
{
public:
    using box_type = box2d<double>;
    using params_type = label_placement::placement_params;

    single_line_layout(params_type const & params);

    template <typename Detector>
    bool try_placement(
        text_layout_generator & layout_generator,
        Detector & detector,
        vertex_cache & path);

    inline double get_length(text_layout_generator const & layout_generator) const
    {
        return layout_generator.layouts_->width();
    }

private:
    template <typename Detector>
    bool try_placement(
        layout_container const & layouts,
        Detector & detector,
        evaluated_text_properties const & text_props,
        vertex_cache &pp,
        text_upright_e orientation,
        glyph_positions & glyphs,
        std::vector<box_type> & bboxes);

    template <typename Detector>
    void process_bboxes(
        Detector & detector,
        layout_container & layouts,
        glyph_positions_ptr & glyphs,
        std::vector<box_type> const & bboxes);

    template <typename Detector>
    bool collision(
        Detector & detector,
        evaluated_text_properties const & text_props,
        box_type const& box,
        const value_unicode_string &repeat_key,
        bool line_placement) const;

    params_type const & params_;
};

}//ns mapnik

#endif // LINE_LAYOUT_HPP
