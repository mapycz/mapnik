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

// mapnik
#include <mapnik/feature.hpp>
#include <mapnik/debug.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_helpers.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/agg/render_polygon_pattern.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/vertex_converters.hpp>
#include <mapnik/vertex_processor.hpp>
#include <mapnik/parse_path.hpp>
#include <mapnik/symbolizer.hpp>
#include <mapnik/svg/svg_converter.hpp>
#include <mapnik/svg/svg_renderer_agg.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/renderer_common/render_pattern.hpp>

#pragma GCC diagnostic push
#include <mapnik/warning_ignore_agg.hpp>
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
// for polygon_pattern_symbolizer
#include "agg_renderer_scanline.h"
#include "agg_span_allocator.h"
#include "agg_span_pattern_rgba.h"
#include "agg_image_accessors.h"
#include "agg_conv_clip_polygon.h"
#pragma GCC diagnostic pop

namespace mapnik {

class wrap_mode_alternating
{
public:
    wrap_mode_alternating(unsigned width, unsigned height) :
        m_width(width),
        m_height(height),
        m_width2(width / 2),
        m_value_x(0),
        m_value_y(0),
        m_odd(true)
    {
    }

    inline unsigned x(int v)
    {
        m_value_x = unsigned(v) % m_width;
        if (m_odd)
        {
            return m_value_x;
        }
        return m_value_x + ((m_value_x >= m_width2) ? (-1) : 1) * m_width2;
    }

    inline unsigned inc_x()
    {
        ++m_value_x;
        if(m_value_x > m_width) m_value_x = 0;
        if (m_odd)
        {
            if(m_value_x >= m_width) m_value_x = 0;
            return m_value_x;
        }
        return m_value_x + ((m_value_x >= m_width2) ? (-1) : 1) * m_width2;
    }

    inline unsigned y(int v)
    {
        m_odd = ((v / m_height) % 2) == 1;
        return m_value_y = unsigned(v) % m_height;
    }

    inline unsigned inc_y()
    {
        ++m_value_y;
        if(m_value_y >= m_height) m_value_y = 0;
        return m_value_y;
    }

private:
    unsigned m_width;
    unsigned m_height;
    unsigned m_width2;
    unsigned m_value_x;
    unsigned m_value_y;
    bool m_odd;
};

template<class PixFmt, class Wrap> class image_accessor_wrap
{
public:
    typedef PixFmt   pixfmt_type;
    typedef typename pixfmt_type::color_type color_type;
    typedef typename pixfmt_type::order_type order_type;
    typedef typename pixfmt_type::value_type value_type;
    enum pix_width_e { pix_width = pixfmt_type::pix_width };

    image_accessor_wrap() {}
    explicit image_accessor_wrap(const pixfmt_type& pixf) :
        m_pixf(&pixf),
        m_wrap(pixf.width(), pixf.height())
    {}

    void attach(const pixfmt_type& pixf)
    {
        m_pixf = &pixf;
    }

    inline const agg::int8u* span(int x, int y, unsigned)
    {
        m_x = x;
        m_row_ptr = m_pixf->row_ptr(m_wrap.y(y));
        return m_row_ptr + m_wrap.x(x) * pix_width;
    }

    inline const agg::int8u* next_x()
    {
        int x = m_wrap.inc_x();
        return m_row_ptr + x * pix_width;
    }

    inline const agg::int8u* next_y()
    {
        m_row_ptr = m_pixf->row_ptr(m_wrap.inc_y());
        return m_row_ptr + m_wrap.x(m_x) * pix_width;
    }

private:
    const pixfmt_type* m_pixf;
    const agg::int8u*       m_row_ptr;
    int                m_x;
    Wrap               m_wrap;
};


template <typename T0, typename T1>
void agg_renderer<T0,T1>::process(polygon_pattern_symbolizer const& sym,
                                  mapnik::feature_impl & feature,
                                  proj_transform const& prj_trans)
{
    std::string filename = get<std::string, keys::file>(sym, feature, common_.vars_);
    std::shared_ptr<mapnik::marker const> marker = marker_cache::instance().find(filename, true);
    if (marker->is<marker_null>())
    {
        return;
    }

    buffer_type & current_buffer = buffers_.top().get();
    agg::rendering_buffer buf(current_buffer.bytes(), current_buffer.width(),
                              current_buffer.height(), current_buffer.row_size());
    ras_ptr->reset();
    value_double gamma = get<value_double, keys::gamma>(sym, feature, common_.vars_);
    gamma_method_enum gamma_method = get<gamma_method_enum, keys::gamma_method>(sym, feature, common_.vars_);
    if (gamma != gamma_ || gamma_method != gamma_method_)
    {
        set_gamma_method(ras_ptr, gamma, gamma_method);
        gamma_method_ = gamma_method;
        gamma_ = gamma;
    }

    using vertex_converter_type = vertex_converter<transform2_tag,
                                                   clip_poly_tag,
                                                   transform_tag,
                                                   affine_transform_tag,
                                                   simplify_tag,
                                                   smooth_tag>;
    using pattern_type = agg_polygon_pattern<vertex_converter_type>;

    common_pattern_process_visitor visitor(common_, sym, feature);
    image_rgba8 image(util::apply_visitor(visitor, *marker));

    pattern_type pattern(image, common_, sym, feature, prj_trans);

    pattern_type::pixfmt_type pixf(buf);
    pixf.comp_op(static_cast<agg::comp_op_e>(get<composite_mode_e, keys::comp_op>(sym, feature, common_.vars_)));
    pattern_type::renderer_base renb(pixf);

    unsigned w = image.width();
    unsigned h = image.height();
    agg::rendering_buffer pattern_rbuf((agg::int8u*)image.bytes(),w,h,w*4);
    agg::pixfmt_rgba32_pre pixf_pattern(pattern_rbuf);
    pattern_type::img_source_type img_src(pixf_pattern);

    if (pattern.clip_ && !pattern.prj_trans_.equal())
    {
        pattern.converter_.template set<transform2_tag>();
    }
    else
    {
        pattern.converter_.template set<transform_tag>();
    }

    if (pattern.clip_) pattern.converter_.set<clip_poly_tag>();

    ras_ptr->filling_rule(agg::fill_even_odd);

    pattern.render(renb, *ras_ptr);
}

template void agg_renderer<image_rgba8>::process(polygon_pattern_symbolizer const&,
                                                 mapnik::feature_impl &,
                                                 proj_transform const&);

}
