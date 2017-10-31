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

#ifdef HAVE_LIBXML2

// mapnik
#include <mapnik/xml_loader.hpp>
#include <mapnik/xml_node.hpp>
#include <mapnik/config_error.hpp>
#include <mapnik/util/trim.hpp>
#include <mapnik/util/noncopyable.hpp>
#include <mapnik/util/fs.hpp>

// libxml
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/parserInternals.h>
#include <libxml/xinclude.h>

// libxslt
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>
#include <libexslt/exslt.h>

// stl
#include <stdexcept>
#include <mutex>

#define DEFAULT_OPTIONS (XML_PARSE_NOENT | XML_PARSE_NOBLANKS | XML_PARSE_DTDLOAD | XML_PARSE_NOCDATA)

#define isnbsp(s)       ((s)[0] == '\xC2' && (s)[1] == '\xA0')


namespace mapnik
{
class libxml2_loader : util::noncopyable
{
    struct xml_error : public config_error
    {
        xml_error(xmlError const & error)
            : config_error(formatMessage(error), error.line, error.file)
        {
        }

        std::string formatMessage(xmlError const & error) const
        {
            std::string msg("XML document not well formed: ");
            msg += error.message;
            // remove CR
            msg = msg.substr(0, msg.size() - 1);
            return msg;
        }
    };

public:
    libxml2_loader(const char *encoding = nullptr, int options = DEFAULT_OPTIONS, const char *url = nullptr) :
        ctx_(0),
        encoding_(encoding),
        options_(options),
        url_(url)
    {
        LIBXML_TEST_VERSION;
        ctx_ = xmlNewParserCtxt();
        if (!ctx_)
        {
            throw std::runtime_error("Failed to create parser context.");
        }

	{
#ifdef MAPNIK_THREADSAFE
		std::lock_guard<std::mutex> lock(mutex_);
#endif
		exsltRegisterAll();
	}
    }

    ~libxml2_loader()
    {
        if (ctx_)
        {
            xmlFreeParserCtxt(ctx_);
        }
    }

    void load(std::string const& filename, xml_node &node)
    {
        if (!mapnik::util::exists(filename))
        {
            throw config_error(std::string("Could not load map file: File does not exist"), 0, filename);
        }

        xmlDocPtr doc = xmlCtxtReadFile(ctx_, filename.c_str(), encoding_, options_);

        load(doc, node);
    }

    void load(const int fd, xml_node &node)
    {
        xmlDocPtr doc = xmlCtxtReadFd(ctx_, fd, url_, encoding_, options_);

        load(doc, node);
    }

    void load_string(std::string const& buffer, xml_node &node, std::string const & base_path)
    {
        if (!base_path.empty())
        {
            if (!mapnik::util::exists(base_path)) {
                throw config_error(std::string("Could not locate base_path '") +
                                   base_path + "': file or directory does not exist");
            }
        }
        // NOTE: base_path here helps libxml2 resolve entities correctly: https://github.com/mapnik/mapnik/issues/440
        xmlDocPtr doc = xmlCtxtReadMemory(ctx_, buffer.data(), buffer.length(), base_path.c_str(), encoding_, options_);

        load(doc, node);
    }

    xmlDocPtr preprocess(xmlDocPtr doc)
    {
        xsltStylesheetPtr style = xsltLoadStylesheetPI(doc);

        if (style)
        {
            xsltTransformContextPtr transform_ctx = xsltNewTransformContext(style, doc);

            if (transform_ctx)
            {
                xsltSetCtxtParseOptions(transform_ctx, options_);
                xmlDocPtr res = xsltApplyStylesheetUser(style, doc, NULL, NULL, NULL, transform_ctx);
                xsltFreeTransformContext(transform_ctx);
                xsltFreeStylesheet(style);
                return res;
            }

            xsltFreeStylesheet(style);
        }
        return NULL;
    }

private:
    void load(xmlDocPtr doc, xml_node &node)
    {
        check_error(doc);

        xmlDocPtr res = preprocess(doc);

        if (res)
        {
            xmlFreeDoc(doc);
            doc = res;
        }

        check_error(doc);

        int iXIncludeReturn = xmlXIncludeProcessFlags(doc, options_);

        if (iXIncludeReturn < 0)
        {
            xmlFreeDoc(doc);
            throw config_error("XML XInclude error.  One or more files failed to load.");
        }

        xmlNode * root = xmlDocGetRootElement(doc);
        if (!root) {
            xmlFreeDoc(doc);
            throw config_error("XML document is empty.");
        }

        populate_tree(root, node);
        xmlFreeDoc(doc);
    }

    void inline append_attributes(xmlAttr *attributes, xml_node & node)
    {
        for (; attributes; attributes = attributes->next )
        {
            node.add_attribute(reinterpret_cast<const char *>(attributes->name),
                               reinterpret_cast<const char *>(attributes->children->content));
        }
    }

    void inline populate_tree(xmlNode *cur_node, xml_node &node)
    {
        for (; cur_node; cur_node = cur_node->next )
        {
            switch (cur_node->type)
            {
            case XML_ELEMENT_NODE:
            {
                xml_node &new_node = node.add_child(reinterpret_cast<const char *>(cur_node->name), cur_node->line, false);
                append_attributes(cur_node->properties, new_node);
                populate_tree(cur_node->children, new_node);
            }
            break;
            case XML_TEXT_NODE:
            {
                std::string trimmed(reinterpret_cast<const char *>(cur_node->content));
                mapnik::util::trim(trimmed);
                if (trimmed.empty()) break; //Don't add empty text nodes
                node.add_child(trimmed.c_str(), cur_node->line, true);
            }
            break;
            case XML_COMMENT_NODE:
                break;
            default:
                break;

            }
        }
    }

    void check_error(xmlDocPtr const & doc) const
    {
        if (!doc)
        {
            if (xmlError * error = xmlCtxtGetLastError(ctx_))
            {
                throw xml_error(*error);
            }
            else
            {
                throw config_error("XML document not well formed");
            }
        }
    }

    xmlParserCtxtPtr ctx_;
    const char *encoding_;
    int options_;
    const char *url_;
#ifdef MAPNIK_THREADSAFE
    std::mutex mutex_;
#endif
};

void read_xml(std::string const & filename, xml_node &node)
{
    libxml2_loader loader;
    loader.load(filename, node);
}
void read_xml_string(std::string const & str, xml_node &node, std::string const & base_path)
{
    libxml2_loader loader;
    loader.load_string(str, node, base_path);
}

} // end of namespace mapnik

#endif
