<!-- Name: OpenSolarisInstallation/TroubleShooting -->
<!-- Version: 7 -->
<!-- Last-Modified: 2010/11/23 07:49:15 -->
<!-- Author: springmeyer -->
# Trouble Shooting on Open Solaris

## Boost Build Errors

### Boost python:  '--> cannot appear in constant expression'

*description*: You get a compile error when building boost python like:

```sh
    ./boost/python/converter/rvalue_from_python_data.hpp:99: error: '->' cannot appear in a constant-expression
```

*solution*: You are likely compiling boost with the default gcc on Open Solaris (`gcc version 3.4.3 (csl-sol210-3_4-20050802)`). The only solution found is to upgrade to gcc 4.4


### Boost python:  'non-static const member const boost::python::type_info'

*description*: You get a compile error when building boost python like:

```sh
    "/usr/gcc/4.3/bin/g++"  -ftemplate-depth-128 -O3 -finline-functions -Wno-inline -Wall -pthreads -fPIC  -DBOOST_ALL_NO_LIB=1 -DBOOST_PYTHON_SOURCE -DNDEBUG  -I"." -I"/usr/include/python2.6" -c -o "bin.v2/libs/python/build/gcc-4.3.3/release/threading-multi/converter/registry.o" "libs/python/src/converter/registry.cpp"
    ./boost/python/converter/registrations.hpp: In member function ‘boost::python::converter::registration& boost::python::converter::registration::operator=(const boost::python::converter::registration&)’:
    ./boost/python/converter/registrations.hpp:35:   instantiated from ‘void __gnu_cxx::_SGIAssignableConcept<_Tp>::__constraints() [with _Tp = boost::python::converter::registration]’
    /usr/gcc/4.3/lib/gcc/i386-pc-solaris2.11/4.3.3/../../../../include/c++/4.3.3/bits/stl_set.h:96:   instantiated from ‘std::set<boost::python::converter::registration, std::less<boost::python::converter::registration>, std::allocator<boost::python::converter::registration> >’
    libs/python/src/converter/registry.cpp:121:   instantiated from here
    ./boost/python/converter/registrations.hpp:35: error: non-static const member ‘const boost::python::type_info boost::python::converter::registration::target_type’, can't use default assignment operator
    ./boost/python/converter/registrations.hpp:35: error: non-static const member ‘const bool boost::python::converter::registration::is_shared_ptr’, can't use default assignment operator
    /usr/gcc/4.3/lib/gcc/i386-pc-solaris2.11/4.3.3/../../../../include/c++/4.3.3/bits/boost_concept_check.h: In member function ‘void __gnu_cxx::_SGIAssignableConcept<_Tp>::__constraints() [with _Tp = boost::python::converter::registration]’:
    /usr/gcc/4.3/lib/gcc/i386-pc-solaris2.11/4.3.3/../../../../include/c++/4.3.3/bits/boost_concept_check.h:209: note: synthesized method ‘boost::python::converter::registration& boost::python::converter::registration::operator=(const boost::python::converter::registration&)’ first required here 
```

*solution*: You are likely compiling boost with the more recent gcc 4.3.3. that can be download like:

    $ pfexec pkg install gcc-43
    # while configuring bjam like:
    using gcc : 4.3.3 : /usr/gcc/4.3/bin/g++ ;

The only solution known is to upgrade to gcc 4.4

## Mapnik Configure errors

*description:* Scons can't "find" C++ apps during configure. But the `config.log` shows some odd errors with g++ like:


    ld: fatal: file /usr/lib/libgcc_s.so: version `GCC_4.2.0' does not exist:
    	required by file /opt/ts/gcc/4.4/lib/gcc/i386-pc-solaris2.11/4.4.4/../../../libstdc++.so

*solution:* Basically, Scons is running compile tests and they fail because g++ is broken because our custom built g++'s c++ standard lib is linked again the system g++, and we need to get it to link its own g++ version. Normally `export LD_LIBRARY_PATH=/opt/ts/gcc/4.4/lib/` would fix this by prioritizing linking, but on solaris LD_LIBRARY_PATH does not take precedence like on linux (read: it's broke). So, the (only known) workaround is to forcefully change the symlink:

```sh
    pfexec rm /usr/lib/libgcc_s.so
    pfexec ln -s /opt/ts/gcc/4.4/lib/libgcc_s.so /usr/lib/libgcc_s.so
```

## Mapnik Compile errors

*description:* When configuring Mapnik SCons fails on boost and the 'config.log' reports:

```
    Undefined                       first referenced
     symbol                             in file
    std::ctype<char>::_M_widen_init() const/usr/local/lib/libboost_regex.so
    std::basic_ostream<char, std::char_traits<char> >& std::basic_ostream<char, std::char_traits<char> >::_M_insert<unsigned long>(unsigned long)/usr/local/lib/libboost_regex.so
    std::basic_ostream<char, std::char_traits<char> >& std::__ostream_insert<char, std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*, int)/usr/local/lib/libboost_regex.so
```

or 

```
    Undefined                       first referenced
     symbol                             in file
    std::bad_alloc::what() const        /usr/local/lib/libboost_iostreams.so
```

*solution:* The Mapnik SCons dependency checks are using an old version of g++ and you need to force gcc44:


    python scons/scons.py CXX=/opt/ts/gcc/4.4/bin/g++

## Mapnik Runtime errors

### Locale name not valid error
*description:* Mapnik seems to compile okay but running it immediately results in:


    terminate called after throwing an instance of 'std::runtime_error'
        what(): locale::facet::_S_create_c_locale name not valid

*solution:* Something is broken (perhaps in the dev upgrade) with the locale, so you must set it:


    # setting as C works, POSIX likely will as well:
    export LANG="C"
    export LC_ALL="C"


### PSQL Error: could not receive data from server: Error 0
*description:* Mapnik is able to connect to a postgres/postgis database but queries fail with "Error 0":


    An error occurred: PSQL error:
    could not receive data from server: Error 0
    Full sql was: 'SELECT AsBinary("way") AS geom,"name","place" from 
          (select way,place,name,ref
           from planet_osm_point
           where place in ('country','state')
          ) as placenames WHERE "way" && SetSRID('BOX3D(-135 -45,-90 0)'::box3d, 4326)'

*solution:* Postgres was likely built without thread support, which surprisingly is not on by default, but the majority of packages use. Go back and re-configure and install postgres:


    ./configure --enable-thread-safety
    make && sudo make install
