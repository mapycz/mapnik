/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2011 Artem Pavlenko
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
#include <mapnik/debug.hpp>
#include <mapnik/graphics.hpp>
#include <mapnik/agg_renderer.hpp>
#include <mapnik/agg_rasterizer.hpp>
#include <mapnik/expression_evaluator.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/marker.hpp>
#include <mapnik/marker_cache.hpp>
#include <mapnik/svg/svg_renderer.hpp>
#include <mapnik/svg/svg_path_adapter.hpp>
#include <mapnik/markers_placement.hpp>
#include <mapnik/arrow.hpp>
#include <mapnik/markers_symbolizer.hpp>

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_scanline_p.h"
#include "agg_path_storage.h"
#include "agg_ellipse.h"
#include "agg_conv_stroke.h"
#include "agg_conv_clip_polyline.h"

namespace mapnik {

template <typename T>
void agg_renderer<T>::process(markers_symbolizer const& sym,
                              mapnik::feature_ptr const& feature,
                              proj_transform const& prj_trans)
{
    typedef agg::conv_clip_polyline<geometry_type> clipped_geometry_type;
    typedef coord_transform2<CoordTransform,clipped_geometry_type> path_type;

    typedef agg::pixfmt_rgba32 pixfmt;
    typedef agg::renderer_base<pixfmt> renderer_base;
    typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;

    ras_ptr->reset();
    ras_ptr->gamma(agg::gamma_power());
    agg::scanline_u8 sl;
    agg::scanline_p8 sl_line;
    agg::rendering_buffer buf(pixmap_.raw_data(), width_, height_, width_ * 4);
    pixfmt pixf(buf);
    renderer_base renb(pixf);
    renderer_solid ren(renb);
    agg::trans_affine tr;
    boost::array<double,6> const& m = sym.get_image_transform();
    tr.load_from(&m[0]);
    tr = agg::trans_affine_scaling(scale_factor_) * tr;
    std::string filename = path_processor_type::evaluate(*sym.get_filename(), *feature);
    marker_placement_e placement_method = sym.get_marker_placement();
    marker_type_e marker_type = sym.get_marker_type();
    metawriter_with_properties writer = sym.get_metawriter();

    if (!filename.empty())
    {
        boost::optional<marker_ptr> mark = mapnik::marker_cache::instance()->find(filename, true);
        if (mark && *mark)
        {
            if (!(*mark)->is_vector())
            {
                MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: markers_symbolizer do not yet support SVG markers";

                return;
            }
            boost::optional<path_ptr> marker = (*mark)->get_vector_data();
            box2d<double> const& bbox = (*marker)->bounding_box();
            double center_x = 0.5 * (bbox.minx() + bbox.maxx());
            double center_y = 0.5 * (bbox.miny() + bbox.maxy());
            double w = (*mark)->width();
            double h = (*mark)->height();

            agg::trans_affine recenter = agg::trans_affine_translation(-center_x, -center_y);
            agg::trans_affine recenter_tr = recenter * tr;
            box2d<double> extent = bbox * recenter_tr;

            using namespace mapnik::svg;
            vertex_stl_adapter<svg_path_storage> stl_storage((*marker)->source());
            svg_path_adapter svg_path(stl_storage);
            svg_renderer<svg_path_adapter,
                agg::pod_bvector<path_attributes>,
                renderer_solid,
                agg::pixfmt_rgba32 > svg_renderer(svg_path,(*marker)->attributes());

            for (unsigned i=0; i<feature->num_geometries(); ++i)
            {
                geometry_type & geom = feature->get_geometry(i);
                // TODO - merge this code with point_symbolizer rendering
                if (placement_method == MARKER_POINT_PLACEMENT || geom.num_points() <= 1)
                {
                    double x;
                    double y;
                    double z=0;
                    geom.label_interior_position(&x, &y);
                    prj_trans.backward(x,y,z);
                    t_.forward(&x,&y);
                    extent.re_center(x,y);

                    if (sym.get_allow_overlap() ||
                        detector_->has_placement(extent))
                    {

                        render_marker(pixel_position(x - 0.5 * w, y - 0.5 * h), **mark, tr, sym.get_opacity());

                        // TODO - impl this for markers?
                        //if (!sym.get_ignore_placement())
                        //    detector_->insert(label_ext);
                        metawriter_with_properties writer = sym.get_metawriter();
                        if (writer.first) writer.first->add_box(extent, *feature, t_, writer.second);
                    }
                }
                else
                {
                    clipped_geometry_type clipped(geom);
                    clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
                    path_type path(t_,clipped,prj_trans);
                    markers_placement<path_type, label_collision_detector4> placement(path, extent, *detector_,
                                                                                      sym.get_spacing() * scale_factor_,
                                                                                      sym.get_max_error(),
                                                                                      sym.get_allow_overlap());
                    double x, y, angle;

                    while (placement.get_point(&x, &y, &angle))
                    {
                        agg::trans_affine matrix = recenter_tr;
                        matrix.rotate(angle);
                        matrix.translate(x, y);
                        svg_renderer.render(*ras_ptr, sl, renb, matrix, sym.get_opacity(),bbox);

                        if (/* DEBUG */ 0) {
                            // blue outline showing the box used for collision detection
                            box2d<double> box = extent * agg::trans_affine_rotation(angle);
                            double tx = 0, ty = 0;
                            matrix = agg::trans_affine_rotation(angle);
                            matrix.translate(x, y);
                            matrix.transform(&tx, &ty);
                            // prepare path
                            agg::path_storage pbox;
                            pbox.start_new_path();
                            pbox.move_to(tx + box.minx(), ty + box.miny());
                            pbox.line_to(tx + box.maxx(), ty + box.miny());
                            pbox.line_to(tx + box.maxx(), ty + box.maxy());
                            pbox.line_to(tx + box.minx(), ty + box.maxy());
                            pbox.line_to(tx + box.minx(), ty + box.miny());
                            // draw outline
                            agg::conv_stroke<agg::path_storage> sbox(pbox);
                            sbox.generator().width(1.0 * scale_factor_);
                            ras_ptr->reset();
                            ras_ptr->add_path(sbox);
                            ren.color(agg::rgba8(0x33, 0x33, 0xff, 0xcc));
                            agg::render_scanlines(*ras_ptr, sl_line, ren);
                        }

                        if (writer.first)
                        {
                            //writer.first->add_box(label_ext, feature, t_, writer.second);

                            MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: metawriter do not yet supported for line placement";
                        }
                    }
                }
            }
        }
    }
    else // FIXME: should default marker be stored in marker_cache ???
    {
        color const& fill_ = sym.get_fill();
        unsigned r = fill_.red();
        unsigned g = fill_.green();
        unsigned b = fill_.blue();
        unsigned a = fill_.alpha();
        stroke const& stroke_ = sym.get_stroke();
        color const& col = stroke_.get_color();
        double strk_width = stroke_.get_width();
        unsigned s_r=col.red();
        unsigned s_g=col.green();
        unsigned s_b=col.blue();
        unsigned s_a=col.alpha();
        double w = sym.get_width();
        double h = sym.get_height();
        double rx = w/2.0;
        double ry = h/2.0;

        arrow arrow_;
        box2d<double> extent;

        double dx = w + (2*strk_width);
        double dy = h + (2*strk_width);

        if (marker_type == ARROW)
        {
            extent = arrow_.extent() * tr;

            //MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: " << extent << "\n";
        }
        else
        {
            extent.init(-dx, -dy, dx, dy);
            extent *= tr;

            //MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: " << extent << "\n";
        }


        double x;
        double y;
        double z=0;

        agg::path_storage marker;

        for (unsigned i=0; i<feature->num_geometries(); ++i)
        {
            geometry_type & geom = feature->get_geometry(i);
            //if (geom.num_points() <= 1) continue;
            if (placement_method == MARKER_POINT_PLACEMENT || geom.num_points() <= 1)
            {
                geom.label_position(&x,&y);
                prj_trans.backward(x,y,z);
                t_.forward(&x,&y);
                int px = int(floor(x - 0.5 * dx));
                int py = int(floor(y - 0.5 * dy));
                box2d<double> label_ext (px, py, px + dx +1, py + dy +1);

                if (sym.get_allow_overlap() ||
                    detector_->has_placement(label_ext))
                {
                    agg::ellipse c(x, y, rx, ry);
                    marker.concat_path(c);
                    ras_ptr->add_path(marker);
                    ren.color(agg::rgba8(r, g, b, int(a*sym.get_opacity())));
                    // TODO - fill with packed scanlines? agg::scanline_p8
                    // and agg::renderer_outline_aa
                    agg::render_scanlines(*ras_ptr, sl, ren);

                    // outline
                    if (strk_width)
                    {
                        ras_ptr->reset();
                        agg::conv_stroke<agg::path_storage>  outline(marker);
                        outline.generator().width(strk_width * scale_factor_);
                        ras_ptr->add_path(outline);

                        ren.color(agg::rgba8(s_r, s_g, s_b, int(s_a*stroke_.get_opacity())));
                        agg::render_scanlines(*ras_ptr, sl_line, ren);
                    }
                    if (!sym.get_ignore_placement())
                        detector_->insert(label_ext);
                    if (writer.first) writer.first->add_box(label_ext, *feature, t_, writer.second);
                }
            }
            else
            {

                if (marker_type == ARROW)
                    marker.concat_path(arrow_);

                clipped_geometry_type clipped(geom);
                clipped.clip_box(query_extent_.minx(),query_extent_.miny(),query_extent_.maxx(),query_extent_.maxy());
                path_type path(t_,clipped,prj_trans);
                markers_placement<path_type, label_collision_detector4> placement(path, extent, *detector_,
                                                                                  sym.get_spacing() * scale_factor_,
                                                                                  sym.get_max_error(),
                                                                                  sym.get_allow_overlap());
                double x_t, y_t, angle;

                while (placement.get_point(&x_t, &y_t, &angle))
                {
                    agg::trans_affine matrix;

                    if (marker_type == ELLIPSE)
                    {
                        // todo proper bbox - this is buggy
                        agg::ellipse c(x_t, y_t, rx, ry);
                        marker.concat_path(c);
                        agg::trans_affine matrix;
                        matrix *= agg::trans_affine_translation(-x_t,-y_t);
                        matrix *= agg::trans_affine_rotation(angle);
                        matrix *= agg::trans_affine_translation(x_t,y_t);
                        marker.transform(matrix);

                    }
                    else
                    {
                        matrix = tr * agg::trans_affine_rotation(angle) * agg::trans_affine_translation(x_t, y_t);
                    }


                    // TODO
                    if (writer.first)
                    {
                        //writer.first->add_box(label_ext, feature, t_, writer.second);

                        MAPNIK_LOG_DEBUG(agg_renderer) << "agg_renderer: metawriter do not yet supported for line placement";
                    }

                    agg::conv_transform<agg::path_storage, agg::trans_affine> trans(marker, matrix);
                    ras_ptr->add_path(trans);

                    // fill
                    ren.color(agg::rgba8(r, g, b, int(a*sym.get_opacity())));
                    agg::render_scanlines(*ras_ptr, sl, ren);

                    // outline
                    if (strk_width)
                    {
                        ras_ptr->reset();
                        agg::conv_stroke<agg::conv_transform<agg::path_storage, agg::trans_affine> >  outline(trans);
                        outline.generator().width(strk_width * scale_factor_);
                        ras_ptr->add_path(outline);
                        ren.color(agg::rgba8(s_r, s_g, s_b, int(s_a*stroke_.get_opacity())));
                        agg::render_scanlines(*ras_ptr, sl_line, ren);
                    }
                }
            }

        }
    }
}

template void agg_renderer<image_32>::process(markers_symbolizer const&,
                                              mapnik::feature_ptr const&,
                                              proj_transform const&);
}
