# Compiling Mapnik Faster
![source:xkcd.org](http://imgs.xkcd.com/comics/compiling.png)

When you are tired of sword fighting, its time to actually figure out how to compile Mapnik faster.

The use of SCons, templates, and particularly boost::spirit2 grammars, means that for a fairly lightweight library like Mapnik things take a long time to compile.

Mapnik trunk has increased usage of boost::spirit2 and default compile times have gone from around 7-10 minutes to 15-20 (on dual core machine, 4GB mem) using g++ 4.2

But, there are several things you can do to speed things up:

## Trigger SCons to compile in parallel
If you have enough memory (> 2 GB) and >2 cores, tell SCons to compile stuff in parallel:

*Note*: on some systems this may not help will overall speed unless you have >4 GB memory

```sh
    $ python scons/scons.py install -j2
```

## Upgrade your compiler
The more recent compiler you are using, likely the faster the compile. g++ 4.5 should be faster than 4.2 for example, and the new clang++/llvm compilers are about twice as fast.

Here is how to build the latest clang++

```sh
    # grab llvm and clang from svn
    svn co http://llvm.org/svn/llvm-project/llvm/trunk llvm
    cd llvm/tools
    svn co http://llvm.org/svn/llvm-project/cfe/trunk clang
    cd ..
    ./configure --enable-optimized
    make
    sudo make install
```

then compile Mapnik like:

*Note:* this requires the boost >= 1.44. You should be able to use a version of boost compiled with g++, but for the advanced users you should also be able to compile boost with clang. See: http://blog.llvm.org/2010/05/clang-builds-boost.html

```sh
    $ python scons/scons.py install CXX="clang++"
```

Be aware that clang provides more warnings that gcc, and this can clog your terminal sometimes when the boost guys get careless. To silence most of the clang warnings that come from boost headers you can do:

```sh
    $ python scons/scons.py install WARNING_CXXFLAGS="-Wno-unused-function -Wno-uninitialized -Wno-array-bounds -Wno-parentheses -Wno-char-subscripts -Wno-internal-linkage-in-inline"
```

## Use Precompiled Headers

Read up on precompiled headers [here](http://clang.llvm.org/docs/UsersManual.html#precompiledheaders) and [here](http://gcc.gnu.org/onlinedocs/gcc-4.0.4/gcc/Precompiled-Headers.html)

If done right this can save about 1/4 of the compile time, sometimes a bit more. On my machine this dropped compilation of libmapnik.dylib from 6 minutes to 4 minutes.

The basic idea here is to determine one or many headers that are not changing, expensive to compile, and frequently includes (directly or indirectly).

So, in the case of Mapnik, boost spirit headers fit this. Boost spirit is all about compile time optimization through C++ templates for fast parsing. It is only used in a few places in mapnik, but the headers that use spirit are then included in many other files, which are includes by other files.... and so on.

Then, the goal is to precompile some or all of boost spirit so that each time a mapnik cpp file is compiled all the references to boost spirit will be pulled from a single pre-compiled header instead of compiled every time.

First we need to develop a list of boost spirit headers and dump them into a single header:

```python
    from glob import glob
    
    hpps = glob('include/mapnik/*hpp')
    hpps.extend(glob('include/mapnik/svg/*hpp'))
    hpps.extend(glob('include/mapnik/grid/*hpp'))
    hpps.extend(glob('include/mapnik/wkt/*hpp'))
    hpps.extend(glob('src/*cpp'))
    hpps.extend(glob('src/svg/*cpp'))
    hpps.extend(glob('src/wkt/*cpp'))
    hpps.extend(glob('src/grid/*cpp'))
    includes = []
    
    for h in hpps:
        f_ = open(h,'r')
        for l in f_.readlines():
            i = l.strip()
            if i.startswith('#include <boost/spirit') or i.startswith('#include <boost/fusion'):
                if not i in includes:
                    includes.append(i)
    
    
    sorted(includes)
    print '\n'.join(includes)
```

Save that script as `parse_spirit_headers.py` and run like:

```sh
    python parse_spirit_headers.py > spirit.h
```

Then compile that header:

Note: a tricky issue here is that the -DEFINES used to compile this header must exactly match those to compile mapnik, which is not always going to be the case.

To get the right defines, start compiling normal and copy/paste the defined from the first src/mapnik*.cpp compiled, and save these in a $CFLAGS variable in the shell.

Then compile the header:

    clang++ -x c++-header $CFLAGS spirit.h -o mapnik.h.pch


Next, in src/SConscript on the line below `lib_env = env.Close()` do:

```python
    lib_env['CXX'] = "clang++ -include-pch mapnik.h.pch"
```

Finally, compile mapnik as normal.

## Use ccache

*Note*: it appears the with recent clang version ccache has no real benefit.

[ccache](http://ccache.samba.org/) is a compiler cache. Basically it a program that runs in front of your existing compiler and tries to help speed it up. It should be able to quarter your compile times after it warms up. E.g. the first compile will be slower but if you are doing development then recompiles afterword should be much faster.

To use ccache, first install it, then compile Mapnik like:

If using clang, you should do:

```sh
    $ python scons/scons.py install CXX="ccache clang++ -Qunused-arguments -fcolor-diagnostics"
```

Or with gcc do:

```sh
    $ python scons/scons.py install CXX="ccache g++"
```

## Using ccache with clang++

One a 13-inch 2.66 GHz Intel Core 2 Duo 4GB macbook using both ccache and clang++ compiled Mapnik from scratch in *15 minutes* the first run, then *5* minutes the second.

## Reduce latency in SCons
Scons is really good - almost too good - at scanning for dependency trees, to ensure quality builds. For example, you could make no change to your Mapnik code checkout, but if you re-install the boost libraries, when you return to install Mapnik likely all of Mapnik will be triggered for recompile. This is a good thing for consistent binaries, but it means that it takes SCons a long time to get started actually sending commands to the compiler.

There are a [variety of tricks](http://www.scons.org/wiki/GoFastButton) but the easiest, and should be safe, is to send the FAST=True flag to Scons, which will slightly lessen its hunger for scanning and should reduce the time it takes to start compiling:

```sh
    $ scons FAST=True
```