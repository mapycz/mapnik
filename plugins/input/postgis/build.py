#
# This file is part of Mapnik (c++ mapping toolkit)
#
# Copyright (C) 2015 Artem Pavlenko
#
# Mapnik is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
#
#

Import ('plugin_base')
Import ('env')
from copy import copy
from glob import glob
import os

PLUGIN_NAME = 'postgis'

plugin_env = plugin_base.Clone()

plugin_sources = Split(
  """
  %(PLUGIN_NAME)s_datasource.cpp
  %(PLUGIN_NAME)s_featureset.cpp
  """ % locals()
)

cxxflags = []
plugin_env['LIBS'] = []

if env['RUNTIME_LINK'] == 'static':
    # pkg-config is more reliable than pg_config across platforms
    cmd = 'pkg-config libpq --libs --static'
    try:
        plugin_env.ParseConfig(cmd)
    except OSError as e:
        plugin_env.Append(LIBS='pq')
else:
    plugin_env.Append(LIBS='pq')

# Link Library to Dependencies
libraries = copy(plugin_env['LIBS'])

if env['PLUGIN_LINKING'] == 'shared':
    libraries.insert(0,env['MAPNIK_NAME'])
    libraries.append('icui18n')
    libraries.append(env['ICU_LIB_NAME'])
    libraries.append(env['BOOST_LIB_PATHS']['system'])
    libraries.append(env['BOOST_LIB_PATHS']['regex'])
    libraries.append('icudata')

    TARGET = plugin_env.SharedLibrary('../%s' % PLUGIN_NAME,
                                      SHLIBPREFIX='',
                                      SHLIBSUFFIX='.input',
                                      source=plugin_sources,
                                      LIBS=libraries)

    # if the plugin links to libmapnik ensure it is built first
    Depends(TARGET, env.subst('../../../src/%s' % env['MAPNIK_LIB_NAME']))

    if 'uninstall' not in COMMAND_LINE_TARGETS:
        env.Install(env['MAPNIK_INPUT_PLUGINS_DEST'], TARGET)
        env.Alias('install', env['MAPNIK_INPUT_PLUGINS_DEST'])

if 'install' in COMMAND_LINE_TARGETS:
    includes = glob('./*.hpp')
    inc_target = os.path.normpath(env['INSTALL_PREFIX'] + '/include/mapnik/plugins/input/')
    env.Alias(target='install', source=env.Install(inc_target, includes))

plugin_obj = {
  'LIBS': libraries,
  'SOURCES': plugin_sources,
}

Return('plugin_obj')
