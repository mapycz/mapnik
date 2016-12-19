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
#ifndef MAPNIK_MARKER_LINE_LAYOUT_HPP
#define MAPNIK_MARKER_LINE_LAYOUT_HPP

#include <mapnik/text/line_layout.hpp>
#include <mapnik/marker_line_policy.hpp>

namespace mapnik
{

template <typename SubLayout>
class marker_line_layout : line_layout<SubLayout>
{
public:
    using params_type = label_placement::placement_params;

    marker_line_layout(params_type const & params)
        : line_layout<SubLayout>(params),
          spacing_(get_spacing()),
          position_tolerance_(spacing_ * params.get<value_double, keys::max_error>())
    {
    }

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom & geom)
    {
        double layout_width = this->sublayout_.get_length(layout_generator);
        vertex_cache path(geom);
        marker_line_policy policy(path, layout_generator,
            layout_width, spacing_, position_tolerance_, this->params_);
        return line_layout<SubLayout>::try_placement(
            layout_generator, detector, path, policy);
    }

protected:
    double get_spacing()
    {
        double spacing = this->params_.get<value_double, keys::spacing>();
        return spacing < 1 ? 100 : spacing;
    }

    const double spacing_;
    const double position_tolerance_;
};

// =======================================


template <typename SubLayout>
class marker_vertex_converter : util::noncopyable
{
    template <typename Layout, typename LayoutGenerator, typename Detector>
    struct converter_adapter
    {
        converter_adapter(Layout & layout,
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
        smooth_tag,
        offset_transform_tag>;

    marker_vertex_converter(params_type const & params)
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
        value_double offset = params.get<value_double, keys::offset>();

        if (clip) converter_.template set<clip_line_tag>();
        converter_.template set<transform_tag>();
        converter_.template set<affine_transform_tag>();
        if (extend > 0.0) converter_.template set<extend_tag>();
        if (simplify_tolerance > 0.0) converter_.template set<simplify_tag>();
        if (smooth > 0.0) converter_.template set<smooth_tag>();
        if (std::fabs(offset) > 0.0) converter_.template set<offset_transform_tag>();
    }

    template <typename LayoutGenerator, typename Detector, typename Geom>
    bool try_placement(
        LayoutGenerator & layout_generator,
        Detector & detector,
        Geom & geom)
    {
        using adapter_type = converter_adapter<SubLayout, LayoutGenerator, Detector>;
        using visitor_type = line_placement_visitor<adapter_type, vertex_converter_type>;
        adapter_type adapter(sublayout_, layout_generator, detector);
        visitor_type visitor(converter_, adapter);
        return util::apply_visitor(visitor, geom);
    }

private:
    SubLayout sublayout_;
    vertex_converter_type converter_;
};


}//ns mapnik

#endif // MAPNIK_MARKER_LINE_LAYOUT_HPP
