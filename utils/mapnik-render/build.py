import os
import glob
from copy import copy

Import ('env')
program_env = env.Clone()

source = Split(
    """
    mapnik-render.cpp
    """
    )

program_env['CXXFLAGS'] = copy(env['LIBMAPNIK_CXXFLAGS'])
program_env.Append(CPPDEFINES = env['LIBMAPNIK_DEFINES'])

if env['HAS_CAIRO']:
    program_env.PrependUnique(CPPPATH=env['CAIRO_CPPPATHS'])
    program_env.Append(CPPDEFINES = '-DHAVE_CAIRO')

libraries = [env['MAPNIK_NAME']]
libraries.append(env['BOOST_LIB_PATHS']['program_options'])
libraries.extend(copy(env['LIBMAPNIK_LIBS']))
if env['RUNTIME_LINK'] == 'static' and env['PLATFORM'] == 'Linux':
    libraries.append('dl')

mapnik_render = program_env.Program('mapnik-render', source, LIBS=libraries)
Depends(mapnik_render, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(os.path.join(env['INSTALL_PREFIX'],'bin'), mapnik_render)
    env.Alias('install', os.path.join(env['INSTALL_PREFIX'],'bin'))

env['create_uninstall_target'](env, os.path.join(env['INSTALL_PREFIX'],'bin','mapnik-render'))
