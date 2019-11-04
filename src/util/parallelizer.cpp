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

#include <mapnik/util/parallelizer.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/image_scaling.hpp>

#include <future>

#ifdef MAPNIK_STATS_RENDER
struct log_operation
{
    std::stringstream log_stream;

    ~log_operation()
    {
        std::string label("Parallel layer: ");
        std::string log_value(log_stream.str());
        std::string::size_type pos = (label.size() > log_value.size()) ?
            0 :  label.size();
        std::clog << log_value.replace(0, pos, label);
    }
};
#endif

namespace mapnik { namespace parallelizer {

MAPNIK_DECL bool is_parallelizable(Map const& map)
{
    parameters const & params = map.get_extra_parameters();
    boost::optional<value_bool> parallel = params.get<value_bool>("parallel");
    return parallel && *parallel;
}

boost::optional<value_double> layer_scale_factor(layer const& lyr)
{
    parameters const & params = lyr.get_extra_parameters();
    return params.get<value_double>("scale-factor");
}

scaling_method_e layer_scaling_method(layer const& lyr)
{
    parameters const & params = lyr.get_extra_parameters();
    boost::optional<std::string> method_name(
        params.get<std::string>("scaling"));
    const scaling_method_e default_method = SCALING_NEAR;
    if (!method_name)
    {
        return default_method;
    }
    boost::optional<scaling_method_e> scaling_method =
        scaling_method_from_string(*method_name);
    return scaling_method ? *scaling_method : default_method;
}

bool is_layer_scale_factor_active(layer const& lyr, double scale_denom)
{
    parameters const & params = lyr.get_extra_parameters();
    boost::optional<value_double> min(params.get<value_double>(
        "scale-factor-minimum-scale-denominator"));
    boost::optional<value_double> max(params.get<value_double>(
        "scale-factor-maximum-scale-denominator"));
    return !((min && scale_denom < *min) || (max && scale_denom > *max));
}

image_rgba8 render_layer(Map map,
                         layer const& lay,
                         projection proj,
                         double scale_denom,
                         double scale_factor,
                         std::size_t width,
                         std::size_t height,
                         std::size_t index)
{
    boost::optional<value_double> lay_scale_factor = layer_scale_factor(lay);
    if (lay_scale_factor &&
        std::abs(*lay_scale_factor - scale_factor) > 0.1 &&
        is_layer_scale_factor_active(lay, scale_denom))
    {
        double scale_factor_ratio = *lay_scale_factor / scale_factor;
        width = std::round(scale_factor_ratio * width);
        height = std::round(scale_factor_ratio * height);
        scale_factor = *lay_scale_factor;
        map.resize(width, height);
    }

    image_rgba8 img(width, height);
    mapnik::agg_renderer<image_rgba8> ren(map, img, scale_factor);

    if (index == 0)
    {
        ren.start_map_processing(map);
    }
    else
    {
        mapnik::set_premultiplied_alpha(img, true);
    }

    layer lay_copy(lay);
    lay_copy.reset_comp_op();
    lay_copy.set_opacity(1.0);

    std::set<std::string> names;
    ren.apply_to_layer(lay_copy,
                       ren,
                       proj,
                       map.scale(),
                       scale_denom,
                       width,
                       height,
                       map.get_current_extent(),
                       map.buffer_size(),
                       names);

    return img;
}

struct layer_job
{
    using future_type = std::future<image_rgba8>;

    layer_job(future_type && future, layer const& lay)
        : future(std::move(future)), lay(lay)
    {
    }

    future_type future;
    layer const& lay;
};

void scale_if_needed(layer const& lay,
                     image_rgba8 & layer_img,
                     std::size_t width,
                     std::size_t height)
{
    if (layer_img.width() != width ||
        layer_img.height() != height)
    {
        image_rgba8 scaled(width, height, true, true);
        scaling_method_e scaling_method = layer_scaling_method(lay);
#ifdef MAPNIK_STATS_RENDER
        log_operation log;
#endif
        scale_image_agg(scaled, layer_img, scaling_method,
            static_cast<double>(width) / layer_img.width(),
            static_cast<double>(height) / layer_img.height(),
            0, 0, 1
#ifdef MAPNIK_STATS_RENDER
            , &log.log_stream
#endif
            );
        scaled.painted(layer_img.painted());
        layer_img = std::move(scaled);
    }
}

MAPNIK_DECL void render(Map const& map,
            image_rgba8 & img,
            double scale_denom,
            double scale_factor)
{
    mapnik::set_premultiplied_alpha(img, true);

    std::vector<layer_job> layer_jobs;
    layer_jobs.reserve(map.layers().size());

    projection proj(map.srs(), true);
    if (scale_denom <= 0.0)
    {
        scale_denom = mapnik::scale_denominator(map.scale(), proj.is_geographic());
    }
    scale_denom *= scale_factor;

    std::size_t index = 0;

    for (auto const& lay : map.layers())
    {
        if (!lay.visible(scale_denom))
        {
            continue;
        }

        layer_jobs.emplace_back(std::async(std::launch::async,
                                           render_layer,
                                           map,
                                           std::cref(lay),
                                           proj,
                                           scale_denom,
                                           scale_factor,
                                           img.width(),
                                           img.height(),
                                           index), lay);
        ++index;
    }

    for (auto & lj : layer_jobs)
    {
        layer const& lay = lj.lay;
        image_rgba8 layer_img(std::move(lj.future.get()));
        scale_if_needed(lay, layer_img, img.width(), img.height());
        composite_mode_e comp_op = lay.comp_op() ? *lay.comp_op() : src_over;
#ifdef MAPNIK_STATS_RENDER
        log_operation log;
#endif
        composite(img, layer_img, comp_op, lay.get_opacity(), 0, 0
#ifdef MAPNIK_STATS_RENDER
                  , &log.log_stream
#endif
                 );
        img.painted(img.painted() || layer_img.painted());
    }

    mapnik::demultiply_alpha(img);
}

} }
