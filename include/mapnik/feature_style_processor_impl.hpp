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

// NOTE: This is an implementation header file and is only meant to be included
//    from implementation files. It therefore doesn't have an include guard. To
//    create a custom feature_style_processor, include this file and instantiate
//    the template with the desired template arguments.

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/feature.hpp>
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/query.hpp>
#include <mapnik/datasource.hpp>
#include <mapnik/feature_type_style.hpp>
#include <mapnik/box2d.hpp>
#include <mapnik/layer.hpp>
#include <mapnik/rule.hpp>
#include <mapnik/rule_cache.hpp>
#include <mapnik/attribute_collector.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/scale_denominator.hpp>
#include <mapnik/projection.hpp>
#include <mapnik/proj_transform.hpp>
#include <mapnik/util/featureset_buffer.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/symbolizer_dispatch.hpp>

// stl
#include <vector>
#include <stdexcept>

namespace mapnik
{

#ifdef MAPNIK_STATS_RENDER
struct painted_features
{
    painted_features(std::string const& layer_name)
        : layer_name(layer_name),
          painted_ids(),
          all_features_count(0)
    {
    }

    std::string layer_name;
    std::set<value_integer> painted_ids;
    unsigned all_features_count = 0;
};

struct log_layer_stats
{
    log_layer_stats(std::string const& layer_name,
                    std::map<std::string, timer> const& datasource_query_times)
        : painted_features_(layer_name),
          datasource_query_times_(datasource_query_times)
    {
    }

    template <typename Timer>
    void operator()(Timer const& t) const
    {
        std::string datasource_time;

        auto dqt_it = datasource_query_times_.find(painted_features_.layer_name);
        if (dqt_it != datasource_query_times_.end())
        {
            datasource_time = dqt_it->second.to_string();
        }

        std::ostringstream s;
        s << datasource_time;
        s << std::setw(31 - (int)s.tellp()) << std::right << "| ";
        s << t.to_string();
        s << std::setw(60 - (int)s.tellp()) << std::right << "| ";
        s << "layer: " << painted_features_.layer_name
            << ", painted features: "
            << painted_features_.painted_ids.size() << "/"
            << painted_features_.all_features_count << std::endl;
        std::clog << s.str();
    }

    painted_features painted_features_;
    std::map<std::string, timer> const& datasource_query_times_;
};

struct log_datasource_query_time
{
    template <typename Timer>
    void operator()(Timer const& t) const
    {
        datasource_query_times[layer_name] = t;
    }

    std::string layer_name;
    std::map<std::string, timer> & datasource_query_times;
};
#endif

// Store material for layer rendering in a two step process
struct layer_rendering_material
{
    layer const& lay_;
    projection const& proj0_;
    projection proj1_;
    box2d<double> layer_ext2_;
    std::vector<feature_type_style const*> active_styles_;
    std::vector<featureset_ptr> featureset_ptr_list_;
    std::vector<rule_cache> rule_caches_;
    std::vector<layer_rendering_material> materials_;

    layer_rendering_material(layer const& lay, projection const& dest)
        :
        lay_(lay),
        proj0_(dest),
        proj1_(lay.srs(),true) {}

    inline bool empty() const
    {
        return active_styles_.empty() && lay_.datasource();
    }

    layer_rendering_material(layer_rendering_material && rhs) = default;
};

template <typename Processor>
feature_style_processor<Processor>::feature_style_processor(Map const& m, double scale_factor)
    : m_(m)
{
#ifdef MAPNIK_STATS_RENDER
    std::clog << "EXTENT: " << m.get_current_extent() << std::endl;
    std::clog << "Datasource query time        | Render time                |" << std::endl;
#endif
    // https://github.com/mapnik/mapnik/issues/1100
    if (scale_factor <= 0)
    {
        throw std::runtime_error("scale_factor must be greater than 0.0");
    }
}

template <typename Processor>
void feature_style_processor<Processor>::prepare_layers(layer_rendering_material & parent_mat,
                                                        std::deque<layer> const & layers,
                                                        feature_style_context_map & ctx_map,
                                                        Processor & p,
                                                        double scale,
                                                        double scale_denom,
                                                        unsigned width,
                                                        unsigned height,
                                                        box2d<double> const& extent,
                                                        int buffer_size)
{
    for (layer const& lyr : layers)
    {
        if (lyr.visible(scale_denom))
        {
            std::set<std::string> names;
            layer_rendering_material mat(lyr, parent_mat.proj0_);

            prepare_layer(mat,
                          ctx_map,
                          p,
                          scale,
                          scale_denom,
                          width,
                          height,
                          extent,
                          buffer_size,
                          names);

            // Store active material
            if (!mat.empty())
            {
                prepare_layers(mat,
                               lyr.layers(),
                               ctx_map,
                               p,
                               scale,
                               scale_denom,
                               width,
                               height,
                               extent,
                               buffer_size);
                parent_mat.materials_.emplace_back(std::move(mat));
            }
        }
    }
}

template <typename Processor>
void feature_style_processor<Processor>::apply(double scale_denom)
{
#ifdef MAPNIK_STATS_RENDER
            mapnik::progress_timer __stats__(std::clog, "map");
#endif

    Processor & p = static_cast<Processor&>(*this);
    p.start_map_processing(m_);

    projection proj(m_.srs(),true);
    if (scale_denom <= 0.0)
        scale_denom = mapnik::scale_denominator(m_.scale(),proj.is_geographic());
    scale_denom *= p.scale_factor(); // FIXME - we might want to comment this out

    // Asynchronous query supports:
    // This is a two steps process,
    // first we setup all queries at layer level
    // in a second time, we fetch the results and
    // do the actual rendering

    // Define processing context map used by datasources
    // implementing asynchronous queries
    feature_style_context_map ctx_map;

    if (!m_.layers().empty())
    {
        layer_rendering_material root_mat(m_.layers().front(), proj);
        prepare_layers(root_mat,
                       m_.layers(),
                       ctx_map, p,
                       m_.scale(),
                       scale_denom,
                       m_.width(),
                       m_.height(),
                       m_.get_current_extent(),
                       m_.buffer_size());

        render_submaterials(root_mat, p);
    }

    p.end_map_processing(m_);
}

template <typename Processor>
void feature_style_processor<Processor>::apply(mapnik::layer const& lyr,
                                               std::set<std::string>& names,
                                               double scale_denom)
{
    Processor & p = static_cast<Processor&>(*this);
    p.start_map_processing(m_);
    projection proj(m_.srs(),true);
    if (scale_denom <= 0.0)
        scale_denom = mapnik::scale_denominator(m_.scale(),proj.is_geographic());
    scale_denom *= p.scale_factor();

    if (lyr.visible(scale_denom))
    {
        apply_to_layer(lyr,
                       p,
                       proj,
                       m_.scale(),
                       scale_denom,
                       m_.width(),
                       m_.height(),
                       m_.get_current_extent(),
                       m_.buffer_size(),
                       names);
    }
    p.end_map_processing(m_);
}

/*!
 * \brief render a layer given a projection and scale.
 */
template <typename Processor>
void feature_style_processor<Processor>::apply_to_layer(layer const& lay,
                                                        Processor & p,
                                                        projection const& proj0,
                                                        double scale,
                                                        double scale_denom,
                                                        unsigned width,
                                                        unsigned height,
                                                        box2d<double> const& extent,
                                                        int buffer_size,
                                                        std::set<std::string>& names)
{
    feature_style_context_map ctx_map;
    layer_rendering_material  mat(lay, proj0);

    prepare_layer(mat,
                  ctx_map,
                  p,
                  scale,
                  scale_denom,
                  width,
                  height,
                  extent,
                  buffer_size,
                  names);

    prepare_layers(mat,
                   lay.layers(),
                   ctx_map,
                   p,
                   scale,
                   scale_denom,
                   width,
                   height,
                   extent,
                   buffer_size);

    if (!mat.empty())
    {
        p.start_layer_processing(mat.lay_, mat.layer_ext2_);

#ifdef MAPNIK_STATS_RENDER
        log_layer_stats lpf(mat.lay_.name(), datasource_query_times_);
        timer_with_action<log_layer_stats> __stats__(lpf);
#endif

        render_material(mat, p
#ifdef MAPNIK_STATS_RENDER
                , lpf.painted_features_
#endif
                );
        render_submaterials(mat, p);

        p.end_layer_processing(mat.lay_);
    }
}

void clip_query_extent(box2d<double> & extent, Map const& map, layer const& lay)
{
    // clip buffered extent by maximum extent, if supplied
    boost::optional<box2d<double>> const& maximum_extent = map.maximum_extent();
    if (maximum_extent)
    {
        extent.clip(*maximum_extent);
    }

    if (!extent.valid())
    {
        return;
    }

    boost::optional<box2d<double>> const& layer_maximum_extent = lay.maximum_extent();
    if (layer_maximum_extent)
    {
        extent.clip(*layer_maximum_extent);
    }
}

void collect_styles_with_comp_op(std::vector<feature_type_style const*> & active_styles,
                                 Map const& map,
                                 layer const& lay,
                                 double scale_denom)
{
    // check for styles needing compositing operations applied
    // https://github.com/mapnik/mapnik/issues/1477
    for (std::string const& style_name : lay.styles())
    {
        boost::optional<feature_type_style const&> style=map.find_style(style_name);
        if (!style)
        {
            continue;
        }

        if (style->comp_op() || style->image_filters().size() > 0)
        {
            if (style->active(scale_denom))
            {
                // we'll have to handle compositing ops
                active_styles.push_back(&(*style));
            }
        }
    }
}

template <typename Processor>
void feature_style_processor<Processor>::prepare_layer(layer_rendering_material & mat,
                                                       feature_style_context_map & ctx_map,
                                                       Processor const & p,
                                                       double scale,
                                                       double scale_denom,
                                                       unsigned width,
                                                       unsigned height,
                                                       box2d<double> const& extent,
                                                       int buffer_size,
                                                       std::set<std::string>& names)
{
    layer const& lay = mat.lay_;

    datasource_ptr ds = lay.datasource();
    if (!ds)
    {
        // No datasource means pure compositional or organizational layer.
        return;
    }

    std::deque<std::string> const& style_names = lay.styles();

    std::size_t num_styles = style_names.size();
    if (num_styles == 0)
    {
        MAPNIK_LOG_DEBUG(feature_style_processor)
            << "feature_style_processor: No style for layer=" << lay.name();
        return;
    }

    processor_context_ptr current_ctx = ds->get_context(ctx_map);
    proj_transform prj_trans(mat.proj0_,mat.proj1_);

    box2d<double> query_ext = extent; // unbuffered
    box2d<double> buffered_query_ext(query_ext);  // buffered

    double buffer_padding = 2.0 * scale;
    boost::optional<int> layer_buffer_size = lay.buffer_size();
    if (layer_buffer_size) // if layer overrides buffer size, use this value to compute buffered extent
    {
        buffer_padding *= *layer_buffer_size;
    }
    else
    {
        buffer_padding *= buffer_size;
    }
    buffered_query_ext.width(query_ext.width() + buffer_padding);
    buffered_query_ext.height(query_ext.height() + buffer_padding);

    clip_query_extent(buffered_query_ext, m_, lay);

    std::vector<feature_type_style const*> & active_styles = mat.active_styles_;

    if (!buffered_query_ext.valid())
    {
        return;
    }

    box2d<double> layer_ext = lay.envelope();
    const box2d<double> buffered_query_ext_map_srs = buffered_query_ext;
    bool fw_success = false;
    bool early_return = false;

    // first, try intersection of map extent forward projected into layer srs
    if (prj_trans.forward(buffered_query_ext, PROJ_ENVELOPE_POINTS) && buffered_query_ext.intersects(layer_ext))
    {
        fw_success = true;
        layer_ext.clip(buffered_query_ext);
    }
    // if no intersection and projections are also equal, early return
    else if (prj_trans.equal())
    {
        early_return = true;
    }
    // next try intersection of layer extent back projected into map srs
    else if (prj_trans.backward(layer_ext, PROJ_ENVELOPE_POINTS) && buffered_query_ext_map_srs.intersects(layer_ext))
    {
        layer_ext.clip(buffered_query_ext_map_srs);
        // forward project layer extent back into native projection
        if (! prj_trans.forward(layer_ext, PROJ_ENVELOPE_POINTS))
        {
            MAPNIK_LOG_ERROR(feature_style_processor)
                << "feature_style_processor: Layer=" << lay.name()
                << " extent=" << layer_ext << " in map projection "
                << " did not reproject properly back to layer projection";
        }
    }
    else
    {
        // if no intersection then nothing to do for layer
        early_return = true;
    }

    if (early_return)
    {
        collect_styles_with_comp_op(active_styles, m_, lay, scale_denom);
        return;
    }

    // if we've got this far, now prepare the unbuffered extent
    // which is used as a bbox for clipping geometries
    boost::optional<box2d<double>> const& maximum_extent = m_.maximum_extent();
    if (maximum_extent)
    {
        query_ext.clip(*maximum_extent);
    }

    box2d<double> & layer_ext2 = mat.layer_ext2_;

    layer_ext2 = lay.envelope();
    if (fw_success)
    {
        if (prj_trans.forward(query_ext, PROJ_ENVELOPE_POINTS))
        {
            layer_ext2.clip(query_ext);
        }
    }
    else
    {
        if (prj_trans.backward(layer_ext2, PROJ_ENVELOPE_POINTS))
        {
            layer_ext2.clip(query_ext);
            prj_trans.forward(layer_ext2, PROJ_ENVELOPE_POINTS);
        }
    }

    std::vector<rule_cache> & rule_caches = mat.rule_caches_;
    attribute_collector collector(names);

    // iterate through all named styles collecting active styles and attribute names
    for (std::string const& style_name : style_names)
    {
        boost::optional<feature_type_style const&> style=m_.find_style(style_name);
        if (!style)
        {
            MAPNIK_LOG_ERROR(feature_style_processor)
                << "feature_style_processor: Style='" << style_name
                << "' required for layer='" << lay.name() << "' does not exist.";
            continue;
        }

        std::vector<rule> const& rules = style->get_rules();
        bool active_rules = false;
        rule_cache rc;
        for(rule const& r : rules)
        {
            if (r.active(scale_denom))
            {
                rc.add_rule(r);
                active_rules = true;
                collector(r);
            }
        }
        if (active_rules)
        {
            rule_caches.push_back(std::move(rc));
            active_styles.push_back(&(*style));
        }
    }

    // Don't even try to do more work if there are no active styles.
    if (active_styles.empty())
    {
        return;
    }

    double qw = query_ext.width()>0 ? query_ext.width() : 1;
    double qh = query_ext.height()>0 ? query_ext.height() : 1;
    query::resolution_type res(width/qw,
                               height/qh);

    query q(layer_ext,res,scale_denom,extent);
    q.set_variables(p.variables());

    if (p.attribute_collection_policy() == COLLECT_ALL)
    {
        layer_descriptor lay_desc = ds->get_descriptor();
        for (attribute_descriptor const& desc : lay_desc.get_descriptors())
        {
            q.add_property_name(desc.get_name());
        }
    }
    else
    {
        for (std::string const& name : names)
        {
            q.add_property_name(name);
        }
    }
    q.set_filter_factor(collector.get_filter_factor());

    // Also query the group by attribute
    std::string const& group_by = lay.group_by();
    if (!group_by.empty())
    {
        q.add_property_name(group_by);
    }

#ifdef MAPNIK_STATS_RENDER
    log_datasource_query_time ldqt{ lay.name(), datasource_query_times_ };
    timer_with_action<log_datasource_query_time> __query_stats__(ldqt);
#endif

    bool cache_features = lay.cache_features() && active_styles.size() > 1;

    std::vector<featureset_ptr> & featureset_ptr_list = mat.featureset_ptr_list_;
    if (!group_by.empty() || cache_features)
    {
        featureset_ptr_list.push_back(ds->features_with_context(q,current_ctx));
    }
    else
    {
        for(std::size_t i = 0; i < active_styles.size(); ++i)
        {
            featureset_ptr_list.push_back(ds->features_with_context(q,current_ctx));
        }
    }
}

template <typename Processor>
void feature_style_processor<Processor>::render_submaterials(layer_rendering_material const & parent_mat,
                                                             Processor & p)
{
    for (layer_rendering_material const & mat : parent_mat.materials_)
    {
        if (!mat.empty())
        {
#ifdef MAPNIK_STATS_RENDER
            log_layer_stats lpf(mat.lay_.name(), datasource_query_times_);
            timer_with_action<log_layer_stats> __stats__(lpf);
#endif
            p.start_layer_processing(mat.lay_, mat.layer_ext2_);

            render_material(mat, p
#ifdef MAPNIK_STATS_RENDER
                , lpf.painted_features_
#endif
                );
            render_submaterials(mat, p);

            p.end_layer_processing(mat.lay_);
        }
    }
}

template <typename Processor>
void feature_style_processor<Processor>::render_material(layer_rendering_material const & mat,
                                                         Processor & p
#ifdef MAPNIK_STATS_RENDER
                                                         ,painted_features & pf
#endif
                                                         )
{
    layer const& lay = mat.lay_;
    datasource_ptr ds = lay.datasource();

    if (!ds)
    {
        return;
    }

    std::vector<feature_type_style const*> const & active_styles = mat.active_styles_;
    std::vector<featureset_ptr> const & featureset_ptr_list = mat.featureset_ptr_list_;
    if (featureset_ptr_list.empty())
    {
        // The datasource wasn't queried because of early return
        // but we have to apply compositing operations on styles
        for (feature_type_style const* style : active_styles)
        {
            p.start_style_processing(*style);
            p.end_style_processing(*style);
        }
        return;
    }

    std::vector<rule_cache> const & rule_caches = mat.rule_caches_;
    proj_transform prj_trans(mat.proj0_,mat.proj1_);
    bool cache_features = lay.cache_features() && active_styles.size() > 1;
    std::string group_by = lay.group_by();

    // Render incrementally when the column that we group by changes value.
    if (!group_by.empty())
    {
        featureset_ptr features = *featureset_ptr_list.begin();
        if (features)
        {
            // Cache all features into the memory_datasource before rendering.
            std::shared_ptr<featureset_buffer> cache = std::make_shared<featureset_buffer>();
            feature_ptr feature, prev;

            while ((feature = features->next()))
            {
                if (prev && prev->get(group_by) != feature->get(group_by))
                {
                    // We're at a value boundary, so render what we have
                    // up to this point.
                    std::size_t i = 0;
                    for (feature_type_style const* style : active_styles)
                    {

                        cache->prepare();
                        render_style(p, style,
                                     rule_caches[i],
                                     cache,
                                     prj_trans
#ifdef MAPNIK_STATS_RENDER
                                     ,pf
#endif
                                     );
                        ++i;
                    }
                    cache->clear();
                }
                cache->push(feature);
                prev = feature;
            }

            std::size_t i = 0;
            for (feature_type_style const* style : active_styles)
            {
                cache->prepare();
                render_style(p, style, rule_caches[i], cache, prj_trans
#ifdef MAPNIK_STATS_RENDER
                             ,pf
#endif
                             );
                ++i;
            }
            cache->clear();
        }
    }
    else if (cache_features)
    {
        std::shared_ptr<featureset_buffer> cache = std::make_shared<featureset_buffer>();
        featureset_ptr features = *featureset_ptr_list.begin();
        if (features)
        {
            // Cache all features into the memory_datasource before rendering.
            feature_ptr feature;
            while ((feature = features->next()))
            {

                cache->push(feature);
            }
        }
        std::size_t i = 0;
        for (feature_type_style const* style : active_styles)
        {
            cache->prepare();
            render_style(p, style,
                         rule_caches[i],
                         cache, prj_trans
#ifdef MAPNIK_STATS_RENDER
                         ,pf
#endif
                         );
            ++i;
        }
    }
    // We only have a single style and no grouping.
    else
    {
        std::size_t i = 0;
        std::vector<featureset_ptr>::const_iterator featuresets = featureset_ptr_list.cbegin();
        for (feature_type_style const* style : active_styles)
        {
            featureset_ptr features = *featuresets++;
            render_style(p, style,
                         rule_caches[i],
                         features,
                         prj_trans
#ifdef MAPNIK_STATS_RENDER
                         ,pf
#endif
                         );
            ++i;
        }
    }

#ifdef MAPNIK_STATS_RENDER
    pf.all_features_count /= active_styles.size();
#endif
}

template <typename Processor>
void feature_style_processor<Processor>::render_style(
    Processor & p,
    feature_type_style const* style,
    rule_cache const& rc,
    featureset_ptr features,
    proj_transform const& prj_trans
#ifdef MAPNIK_STATS_RENDER
    ,painted_features & pf
#endif
    )
{
    p.start_style_processing(*style);
    if (!features)
    {
        p.end_style_processing(*style);
        return;
    }
    mapnik::attributes vars = p.variables();
    feature_ptr feature;
    bool was_painted = false;
    while ((feature = features->next()))
    {
#ifdef MAPNIK_STATS_RENDER
        pf.all_features_count++;
#endif
        bool do_else = true;
        bool do_also = false;
        for (rule const* r : rc.get_if_rules() )
        {
            expression_ptr const& expr = r->get_filter();
            value_type result = util::apply_visitor(evaluate<feature_impl,value_type,attributes>(*feature,vars),*expr);
            if (result.to_bool())
            {
                was_painted = true;
#ifdef MAPNIK_STATS_RENDER
                pf.painted_ids.insert(feature->id());
#endif
                do_else=false;
                do_also=true;
                rule::symbolizers const& symbols = r->get_symbolizers();
                if(!p.process(symbols,*feature,prj_trans))
                {
                    for (symbolizer const& sym : symbols)
                    {
                        util::apply_visitor(symbolizer_dispatch<Processor>(p,*feature,prj_trans),sym);
                    }
                }
                if (style->get_filter_mode() == FILTER_FIRST)
                {
                    // Stop iterating over rules and proceed with next feature.
                    do_also=false;
                    break;
                }
            }
        }
        if (do_else)
        {
            for( rule const* r : rc.get_else_rules() )
            {
                was_painted = true;
#ifdef MAPNIK_STATS_RENDER
                pf.painted_ids.insert(feature->id());
#endif
                rule::symbolizers const& symbols = r->get_symbolizers();
                if(!p.process(symbols,*feature,prj_trans))
                {
                    for (symbolizer const& sym : symbols)
                    {
                        util::apply_visitor(symbolizer_dispatch<Processor>(p,*feature,prj_trans),sym);
                    }
                }
            }
        }
        if (do_also)
        {
            for( rule const* r : rc.get_also_rules() )
            {
                was_painted = true;
                rule::symbolizers const& symbols = r->get_symbolizers();
                if(!p.process(symbols,*feature,prj_trans))
                {
                    for (symbolizer const& sym : symbols)
                    {
                        util::apply_visitor(symbolizer_dispatch<Processor>(p,*feature,prj_trans),sym);
                    }
                }
            }
        }
    }
    p.painted(p.painted() | was_painted);
    p.end_style_processing(*style);
}

}
