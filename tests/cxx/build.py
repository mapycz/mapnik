import os
import glob
from copy import copy

Import ('env')

test_env = env.Clone()

if not env['CPP_TESTS']:
    for cpp_test_bin in glob.glob('*-bin'):
        os.unlink(cpp_test_bin)
else:
    test_env['LIBS'] = [env['MAPNIK_NAME']]
    test_env.AppendUnique(LIBS=copy(env['LIBMAPNIK_LIBS']))
    test_env.AppendUnique(LIBS='mapnik-wkt')
    test_env.AppendUnique(LIBS='mapnik-json')
    if env['RUNTIME_LINK'] == 'static' and env['PLATFORM'] == 'Linux':
        test_env.AppendUnique(LIBS='dl')
    test_env.AppendUnique(CXXFLAGS='-g')
    test_env['CXXFLAGS'] = copy(test_env['LIBMAPNIK_CXXFLAGS'])
    test_env.Append(CPPDEFINES = env['LIBMAPNIK_DEFINES'])
    if test_env['HAS_CAIRO']:
        test_env.PrependUnique(CPPPATH=test_env['CAIRO_CPPPATHS'])
        test_env.Append(CPPDEFINES = '-DHAVE_CAIRO')
    test_env_local = test_env.Clone()
    test_program = test_env_local.Program("run", source=glob.glob('*.cpp'))
    Depends(test_program, env.subst('../../src/%s' % env['MAPNIK_LIB_NAME']))
    Depends(test_program, env.subst('../../src/json/libmapnik-json${LIBSUFFIX}'))
    Depends(test_program, env.subst('../../src/wkt/libmapnik-wkt${LIBSUFFIX}'))
    # build locally if installing
    if 'install' in COMMAND_LINE_TARGETS:
        env.Alias('install',test_program)
