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

#ifndef MAPNIK_FEATURE_STYLE_PROCESSOR_HPP
#define MAPNIK_FEATURE_STYLE_PROCESSOR_HPP

// mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/featureset.hpp>
#include <mapnik/config.hpp>
#include <mapnik/feature_style_processor_context.hpp>
#ifdef MAPNIK_STATS_RENDER
#include <mapnik/value_types.hpp>
#include <mapnik/timer.hpp>
#endif

// stl
#include <deque>
#include <set>
#include <string>
#ifdef MAPNIK_STATS_RENDER
#include <deque>
#include <mutex>
#endif

namespace mapnik
{

class Map;
class layer;
class projection;
class proj_transform;
class feature_type_style;
class rule_cache;
struct layer_rendering_material;

enum eAttributeCollectionPolicy
{
    DEFAULT = 0,
    COLLECT_ALL = 1
};

#ifdef MAPNIK_STATS_RENDER
struct painted_features;

// For readable log even with parallel rendering in multiple threads
struct log_sink
{
    std::stringstream stream_;
    static std::mutex mutex_;

    ~log_sink();
};
#endif

template <typename Processor>
class MAPNIK_DECL feature_style_processor
{
public:
    explicit feature_style_processor(Map const& m,
                                     double scale_factor = 1.0);

    /*!
     * \brief apply renderer to all map layers.
     */
    void apply(double scale_denom_override=0.0);

    /*!
     * \brief apply renderer to a single layer, providing pre-populated set of query attribute names.
     */
    void apply(mapnik::layer const& lyr,
               std::set<std::string>& names,
               double scale_denom_override=0.0);

    /*!
     * \brief render a layer given a projection and scale.
     */
    void apply_to_layer(layer const& lay,
                        Processor & p,
                        projection const& proj0,
                        double scale,
                        double scale_denom,
                        unsigned width,
                        unsigned height,
                        box2d<double> const& extent,
                        int buffer_size,
                        std::set<std::string>& names);

private:
    /*!
     * \brief renders a featureset with the given styles.
     */
    void render_style(Processor & p,
                      feature_type_style const* style,
                      rule_cache const& rules,
                      featureset_ptr features,
                      proj_transform const& prj_trans
#ifdef MAPNIK_STATS_RENDER
                      , painted_features & pf
#endif
                      );

    void prepare_layers(layer_rendering_material & parent_mat,
                        std::deque<layer> const & layers,
                        feature_style_context_map & ctx_map,
                        Processor & p,
                        double scale,
                        double scale_denom,
                        unsigned width,
                        unsigned height,
                        box2d<double> const& extent,
                        int buffer_size);

    /*!
     * \brief prepare features for rendering asynchronously.
     */
    void prepare_layer(layer_rendering_material & mat,
                       feature_style_context_map & ctx_map,
                       Processor const & p,
                       double scale,
                       double scale_denom,
                       unsigned width,
                       unsigned height,
                       box2d<double> const& extent,
                       int buffer_size,
                       std::set<std::string>& names);

    /*!
     * \brief render features list queued when they are available.
     */
    void render_material(layer_rendering_material const & mat,
                         Processor & p
#ifdef MAPNIK_STATS_RENDER
                         , painted_features & pf
#endif
                         );

    void render_submaterials(layer_rendering_material const & mat, Processor & p);

    Map const& m_;
#ifdef MAPNIK_STATS_RENDER
protected:
    std::map<std::string, timer> datasource_query_times_;
    log_sink sink_;
#endif
};
}

#endif // MAPNIK_FEATURE_STYLE_PROCESSOR_HPP
