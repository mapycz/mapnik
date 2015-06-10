import os
import glob
from copy import copy

Import ('env')

app_env = env.Clone()
app_env['LIBS'] = [env['MAPNIK_NAME']]
app_env.AppendUnique(LIBS=copy(env['LIBMAPNIK_LIBS']))
if env['RUNTIME_LINK'] == 'static' and env['PLATFORM'] == 'Linux':
    app_env.AppendUnique(LIBS='dl')
app_env.AppendUnique(CXXFLAGS='-g')
app_env['CXXFLAGS'] = copy(app_env['LIBMAPNIK_CXXFLAGS'])
app_env.Append(CPPDEFINES = env['LIBMAPNIK_DEFINES'])
app_env.PrependUnique(CPPPATH=['./'])
app_env.AppendUnique(LIBS='boost_program_options%s' % env['BOOST_APPEND'])
app_env.AppendUnique(LIBS='boost_filesystem%s' % env['BOOST_APPEND'])
test_env_local = app_env.Clone()

source = ["list_fonts.cpp"]
list_fonts = test_env_local.Program('mapnik-list-fonts', source=source)
Depends(list_fonts, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))

if 'uninstall' not in COMMAND_LINE_TARGETS:
    env.Install(os.path.join(env['INSTALL_PREFIX'],'bin'), list_fonts)
    env.Alias('install', os.path.join(env['INSTALL_PREFIX'],'bin'))

env['create_uninstall_target'](env, os.path.join(env['INSTALL_PREFIX'],'bin','mapnik-list-fonts'))
