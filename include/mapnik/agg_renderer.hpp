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

#ifndef MAPNIK_AGG_RENDERER_HPP
#define MAPNIK_AGG_RENDERER_HPP

// mapnik
#include <mapnik/config.hpp>            // for MAPNIK_DECL
#include <mapnik/feature_style_processor.hpp>
#include <mapnik/util/noncopyable.hpp>       // for noncopyable
#include <mapnik/rule.hpp>              // for rule, symbolizers
#include <mapnik/box2d.hpp>     // for box2d
#include <mapnik/view_transform.hpp>    // for view_transform
#include <mapnik/image_compositing.hpp>  // for composite_mode_e
#include <mapnik/pixel_position.hpp>
#include <mapnik/request.hpp>
#include <mapnik/symbolizer_enumerations.hpp>
#include <mapnik/renderer_common.hpp>
#include <mapnik/image_util.hpp>
// stl
#include <memory>
#include <stack>

// fwd declaration to avoid dependence on agg headers
namespace agg { struct trans_affine; }

// fwd declarations to speed up compile
namespace mapnik {
  class Map;
  class feature_impl;
  class feature_type_style;
  class layer;
  class color;
  struct marker;
  class proj_transform;
  struct rasterizer;
  struct rgba8_t;
  template<typename T> class image;
}

namespace mapnik {

template <typename Buffer>
struct agg_renderer_context : util::movable
{
    agg_renderer_context(Buffer & parent_buffer) :
        parent_buffer(parent_buffer),
        current_buffer()
    {
    }

    agg_renderer_context(
        Buffer & parent_buffer,
        unsigned width,
        unsigned height) :
        parent_buffer(parent_buffer),
        current_buffer(new Buffer(width, height))
    {
    }

    Buffer & active_buffer()
    {
        return current_buffer ? *current_buffer : parent_buffer;
    }

    Buffer & parent_buffer;
    std::unique_ptr<Buffer> current_buffer;
};

template <typename Buffer, typename Detector=renderer_common::detector_type>
class MAPNIK_DECL agg_renderer : private util::noncopyable
{

public:
    using buffer_type = Buffer;
    using context_type = agg_renderer_context<buffer_type>;
    //using processor_impl_type = agg_renderer<Buffer>;
    using detector_type = Detector;
    // create with default, empty placement detector
    agg_renderer(Map const& m, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    // create with external placement detector, possibly non-empty
    agg_renderer(Map const &m, std::shared_ptr<detector_type> detector,
                 double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    // pass in mapnik::request object to provide the mutable things per render
    agg_renderer(Map const& m, request const& req, attributes const& vars, double scale_factor=1.0, unsigned offset_x=0, unsigned offset_y=0);
    ~agg_renderer();
    void start_map_processing(Map const& map, context_type & context);
    void end_map_processing(Map const& map, context_type & context);
    context_type start_layer_processing(layer const& lay, box2d<double> const& query_extent, context_type & context);
    void end_layer_processing(layer const& lay, context_type & context);

    context_type start_style_processing(feature_type_style const& st, context_type & context);
    void end_style_processing(feature_type_style const& st, context_type & context);

    void render_marker(
        Buffer & buffer,
        pixel_position const& pos,
        marker const& marker,
        agg::trans_affine const& tr,
        double opacity,
        composite_mode_e comp_op);

    void process(point_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(line_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(line_pattern_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(polygon_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(polygon_pattern_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(raster_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(shield_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(text_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(building_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(markers_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(group_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(debug_symbolizer const& sym,
                 feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(dot_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);
    void process(collision_symbolizer const& sym,
                 mapnik::feature_impl & feature,
                 proj_transform const& prj_trans,
                 context_type & context);

    inline bool process(rule::symbolizers const&,
                        mapnik::feature_impl&,
                        proj_transform const& ,
                        context_type & )
    {
        // agg renderer doesn't support processing of multiple symbolizers.
        return false;
    }

    template <typename Sym>
    void process_marker(
        Sym const& sym,
        mapnik::feature_impl & feature,
        proj_transform const& prj_trans,
        context_type & context);

    void painted(bool painted);
    bool painted();

    inline eAttributeCollectionPolicy attribute_collection_policy() const
    {
        return DEFAULT;
    }

    inline double scale_factor() const
    {
        return common_.scale_factor_;
    }

    inline attributes const& variables() const
    {
        return common_.vars_;
    }
protected:
    template <typename R>
    void debug_draw_box(R& buf, box2d<double> const& extent,
                        double x, double y, double angle = 0.0);
    void debug_draw_box(Buffer & buffer, box2d<double> const& extent,
                        double x, double y, double angle = 0.0);
    void draw_geo_extent(
        Buffer & buffer,
        box2d<double> const& extent,
        mapnik::color const& color);

private:
    const std::unique_ptr<rasterizer> ras_ptr;
    gamma_method_enum gamma_method_;
    double gamma_;
    renderer_common common_;
    void setup(Map const & m, buffer_type & pixmap);
};

extern template class MAPNIK_DECL agg_renderer<image<rgba8_t>>;

} // namespace mapnik

#endif // MAPNIK_AGG_RENDERER_HPP
