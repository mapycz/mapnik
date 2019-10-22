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

#ifndef RENDERER_HPP
#define RENDERER_HPP

// stl
#include <sstream>
#include <iomanip>
#include <fstream>
#include <memory>

// mapnik
#include <mapnik/map.hpp>
#include <mapnik/image_util.hpp>
#include <mapnik/image_reader.hpp>
#include <mapnik/util/variant.hpp>
#include <mapnik/agg_renderer.hpp>

#if defined(HAVE_CAIRO)
#include <mapnik/cairo/cairo_renderer.hpp>
#include <mapnik/cairo/cairo_image_util.hpp>
#ifdef CAIRO_HAS_SVG_SURFACE
#include <cairo-svg.h>
#endif
#ifdef CAIRO_HAS_PS_SURFACE
#include <cairo-ps.h>
#endif
#ifdef CAIRO_HAS_PDF_SURFACE
#include <cairo-pdf.h>
#endif
#endif

#if defined(SVG_RENDERER)
#include <mapnik/svg/output/svg_renderer.hpp>
#endif

// boost
#include <boost/filesystem.hpp>

namespace visual_tests
{

template <typename ImageType>
struct raster_renderer_base
{
    using image_type = ImageType;

    static constexpr const char * ext = ".png";
    static constexpr const bool support_tiles = true;

    unsigned compare(image_type const & actual, boost::filesystem::path const& reference) const
    {
        std::unique_ptr<mapnik::image_reader> reader(mapnik::get_image_reader(reference.string(), "png"));
        if (!reader.get())
        {
            throw mapnik::image_reader_exception("Failed to load: " + reference.string());
        }

        mapnik::image_any ref_image_any = reader->read(0, 0, reader->width(), reader->height());
        ImageType const & reference_image = mapnik::util::get<ImageType>(ref_image_any);

        return mapnik::compare(actual, reference_image, 0, true);
    }

    void save(image_type const & image, boost::filesystem::path const& path) const
    {
        mapnik::save_to_file(image, path.string(), "png32");
    }
};

struct vector_renderer_base
{
    using image_type = std::string;

    static constexpr const bool support_tiles = false;

    unsigned compare(image_type const & actual, boost::filesystem::path const& reference) const
    {
        std::ifstream stream(reference.string().c_str(), std::ios_base::in | std::ios_base::binary);
        if (!stream)
        {
            throw std::runtime_error("Could not open: " + reference.string());
        }
        std::string expected(std::istreambuf_iterator<char>(stream.rdbuf()), std::istreambuf_iterator<char>());
        return std::max(actual.size(), expected.size()) - std::min(actual.size(), expected.size());
    }

    void save(image_type const & image, boost::filesystem::path const& path) const
    {
        std::ofstream file(path.string().c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
        if (!file)
        {
            throw std::runtime_error("Cannot open file for writing: " + path.string());
        }
        file << image;
    }
};

struct agg_renderer : raster_renderer_base<mapnik::image_rgba8>
{
    static constexpr const char * name = "agg";

    image_type render(mapnik::Map const & map, double scale_factor) const
    {
        image_type image(map.width(), map.height());
        mapnik::agg_renderer<image_type> ren(map, image, scale_factor);
        ren.apply();
        return image;
    }
};

#if defined(HAVE_CAIRO)
struct cairo_renderer : raster_renderer_base<mapnik::image_rgba8>
{
    static constexpr const char * name = "cairo";

    image_type render(mapnik::Map const & map, double scale_factor) const
    {
        mapnik::cairo_surface_ptr image_surface(
            cairo_image_surface_create(CAIRO_FORMAT_ARGB32, map.width(), map.height()),
            mapnik::cairo_surface_closer());
        mapnik::cairo_ptr image_context(mapnik::create_context(image_surface));
        mapnik::cairo_renderer<mapnik::cairo_ptr> ren(map, image_context, scale_factor);
        ren.apply();
        image_type image(map.width(), map.height());
        mapnik::cairo_image_to_rgba8(image, image_surface);
        return image;
    }
};

using surface_create_type = cairo_surface_t *(&)(cairo_write_func_t, void *, double, double);

template <surface_create_type SurfaceCreateFunction>
struct cairo_vector_renderer : vector_renderer_base
{
    static cairo_status_t write(void *closure,
                                const unsigned char *data,
                                unsigned int length)
    {
        std::ostringstream & ss = *reinterpret_cast<std::ostringstream*>(closure);
        ss.write(reinterpret_cast<char const *>(data), length);
        return ss ? CAIRO_STATUS_SUCCESS : CAIRO_STATUS_WRITE_ERROR;
    }

    image_type render(mapnik::Map const & map, double scale_factor) const
    {
        std::ostringstream ss(std::stringstream::binary);
        mapnik::cairo_surface_ptr image_surface(
            SurfaceCreateFunction(write, &ss, map.width(), map.height()),
            mapnik::cairo_surface_closer());
        mapnik::cairo_ptr image_context(mapnik::create_context(image_surface));
        mapnik::cairo_renderer<mapnik::cairo_ptr> ren(map, image_context, scale_factor);
        ren.apply();
        cairo_surface_finish(&*image_surface);
        return ss.str();
    }
};

#ifdef CAIRO_HAS_SVG_SURFACE
struct cairo_svg_renderer : cairo_vector_renderer<cairo_svg_surface_create_for_stream>
{
    static constexpr const char * name = "cairo-svg";
    static constexpr const char * ext = ".svg";
};
#endif

#ifdef CAIRO_HAS_PS_SURFACE
struct cairo_ps_renderer : cairo_vector_renderer<cairo_ps_surface_create_for_stream>
{
    static constexpr const char * name = "cairo-ps";
    static constexpr const char * ext = ".ps";
};
#endif

#ifdef CAIRO_HAS_PDF_SURFACE
struct cairo_pdf_renderer : cairo_vector_renderer<cairo_pdf_surface_create_for_stream>
{
    static constexpr const char * name = "cairo-pdf";
    static constexpr const char * ext = ".pdf";
};
#endif
#endif

#if defined(SVG_RENDERER)
struct svg_renderer : vector_renderer_base
{
    static constexpr const char * name = "svg";
    static constexpr const char * ext = ".svg";

    image_type render(mapnik::Map const & map, double scale_factor) const
    {
        std::stringstream ss;
        std::ostream_iterator<char> output_stream_iterator(ss);
        mapnik::svg_renderer<std::ostream_iterator<char>> ren(map, output_stream_iterator, scale_factor);
        ren.apply();
        return ss.str();
    }
};
#endif

template <typename T>
void set_rectangle(T const & src, T & dst, std::size_t x, std::size_t y)
{
    mapnik::box2d<int> ext0(0, 0, dst.width(), dst.height());
    mapnik::box2d<int> ext1(x, y, x + src.width(), y + src.height());

    if (ext0.intersects(ext1))
    {
        mapnik::box2d<int> box = ext0.intersect(ext1);
        for (std::size_t pix_y = box.miny(); pix_y < static_cast<std::size_t>(box.maxy()); ++pix_y)
        {
            typename T::pixel_type * row_to =  dst.get_row(pix_y);
            typename T::pixel_type const * row_from = src.get_row(pix_y - y);

            for (std::size_t pix_x = box.minx(); pix_x < static_cast<std::size_t>(box.maxx()); ++pix_x)
            {
                row_to[pix_x] = row_from[pix_x - x];
            }
        }
    }
}

template <typename Renderer>
class renderer
{
public:
    using renderer_type = Renderer;
    using image_type = typename Renderer::image_type;

    renderer(boost::filesystem::path const & _output_dir, boost::filesystem::path const & _reference_dir, bool _overwrite)
        : ren(), output_dir(_output_dir), reference_dir(_reference_dir), overwrite(_overwrite)
    {
    }

    image_type render(mapnik::Map const & map, double scale_factor) const
    {
        return ren.render(map, scale_factor);
    }

    image_type render(mapnik::Map const & map, double scale_factor, map_size const & tiles) const
    {
        mapnik::Map tile_map(map);
        mapnik::box2d<double> box = tile_map.get_current_extent();
        image_type image(tile_map.width(), tile_map.height());
        tile_map.resize(image.width() / tiles.width, image.height() / tiles.height);
        double tile_box_width = box.width() / tiles.width;
        double tile_box_height = box.height() / tiles.height;
        for (std::size_t tile_y = 0; tile_y < tiles.height; tile_y++)
        {
            for (std::size_t tile_x = 0; tile_x < tiles.width; tile_x++)
            {
                mapnik::box2d<double> tile_box(
                    box.minx() + tile_x * tile_box_width,
                    box.miny() + tile_y * tile_box_height,
                    box.minx() + (tile_x + 1) * tile_box_width,
                    box.miny() + (tile_y + 1) * tile_box_height);
                tile_map.zoom_to_box(tile_box);
                image_type tile(ren.render(tile_map, scale_factor));
                set_rectangle(tile, image, tile_x * tile.width(), (tiles.height - 1 - tile_y) * tile.height());
            }
        }
        return image;
    }

    result report(image_type const & image,
                  std::string const & name,
                  map_size const & size,
                  map_size const & tiles,
                  double scale_factor) const
    {
        boost::filesystem::path reference = reference_dir / image_file_name(name, size, tiles, scale_factor, true);
        bool reference_exists = boost::filesystem::exists(reference);
        result res;

        res.state = reference_exists ? STATE_OK : STATE_OVERWRITE;
        res.name = name;
        res.renderer_name = Renderer::name;
        res.scale_factor = scale_factor;
        res.size = size;
        res.tiles = tiles;
        res.reference_image_path = reference;
        res.diff = reference_exists ? ren.compare(image, reference) : 0;

        if (res.diff)
        {
            boost::filesystem::create_directories(output_dir);
            boost::filesystem::path path = output_dir / image_file_name(name, size, tiles, scale_factor, false);
            res.actual_image_path = path;
            res.state = STATE_FAIL;
            ren.save(image, path);
        }

        if ((res.diff && overwrite) || !reference_exists)
        {
            ren.save(image, reference);
            res.state = STATE_OVERWRITE;
        }

        return res;
    }

private:
    std::string image_file_name(std::string const & test_name,
                                map_size const & size,
                                map_size const & tiles,
                                double scale_factor,
                                bool reference) const
    {
        std::stringstream s;
        s << test_name << '-' << (size.width / scale_factor) << '-' << (size.height / scale_factor) << '-';
        if (tiles.width > 1 || tiles.height > 1)
        {
            s << tiles.width << 'x' << tiles.height << '-';
        }
        s << std::fixed << std::setprecision(1) << scale_factor << '-' << Renderer::name;
        if (reference)
        {
            s << "-reference";
        }
        s << Renderer::ext;
        return s.str();
    }

    const Renderer ren;
    const boost::filesystem::path output_dir;
    const boost::filesystem::path reference_dir;
    const bool overwrite;
};

using renderer_type = mapnik::util::variant<renderer<agg_renderer>
#if defined(HAVE_CAIRO)
                                            ,renderer<cairo_renderer>
#ifdef CAIRO_HAS_SVG_SURFACE
                                            ,renderer<cairo_svg_renderer>
#endif
#ifdef CAIRO_HAS_PS_SURFACE
                                            ,renderer<cairo_ps_renderer>
#endif
#ifdef CAIRO_HAS_PDF_SURFACE
                                            ,renderer<cairo_pdf_renderer>
#endif
#endif
#if defined(SVG_RENDERER)
                                            ,renderer<svg_renderer>
#endif
                                            >;

}

#endif
