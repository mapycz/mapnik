<!-- Name: BrokenExceptions -->
<!-- Version: 6 -->
<!-- Last-Modified: 2010/12/02 14:53:05 -->
<!-- Author: springmeyer -->
In rare cases boost pythons exception handling breaks.

I've seen this twice: once with early versions (~ sept 2010) of clang, and also with 64 bit open solaris.

What happens is that instead of getting a runtime error with some nice message you get a segfault.

Here are the steps to try to guess what is actually causing the problem:

## Using GDB

Run the python executable in gdb:

( in this example we use the full path to apps for clarity, your system will differ )


```sh
    $ gdb /usr/bin/amd64/python
```

You should get some basic gdb startup output:

```sh
    GNU gdb 6.8
    Copyright (C) 2008 Free Software Foundation, Inc.
    License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
    This is free software: you are free to change and redistribute it.
    There is NO WARRANTY, to the extent permitted by law.  Type "show copying"
    and "show warranty" for details.
    This GDB was configured as "i386-pc-solaris2.11"...
    (no debugging symbols found)
```

Now you have a gdb prompt and you need to run your actual program that is crashing. The goal here is to get a backtrace that may indicate the place where the crash is happening.

For example, is it happening in a datasource plugin? If so that would point toward a problem configuring the paths to shapefiles or the database connections for postgres.

Below we run nik2img by typing 'r' then the path to the command and its arguments:


```sh
    (gdb) r /usr/bin/nik2img.py osm.xml image.png
```

You should see a bunch of output like:

```sh
    Starting program: /usr/bin/amd64/python /usr/bin/nik2img.py osm.xml t.png
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    warning: Lowest section in /lib/amd64/libdl.so.1 is .dynamic at 00000000000000b0
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    warning: Lowest section in /lib/amd64/libintl.so.1 is .dynamic at 00000000000000b0
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    (no debugging symbols found)
    warning: Lowest section in /lib/amd64/librt.so.1 is .dynamic at 00000000000000b0
    warning: Lowest section in /lib/amd64/libpthread.so.1 is .dynamic at 00000000000000b0
    warning: Lowest section in /lib/amd64/libthread.so.1 is .dynamic at 00000000000000b0
```

And then if you hit the crash you should see a line like:

    Program received signal SIGSEGV, Segmentation fault.


Now, type 'bt' in the interpreter to get the backtrace:

```sh
    (gdb) bt
    #0  0x0000000000053735 in ?? ()
    #1  0xfffffd7fff2ec5d1 in _Unwind_RaiseException_Body () from /usr/lib/amd64/libc.so.1
    #2  0xfffffd7fff2ec855 in _Unwind_RaiseException () from /usr/lib/amd64/libc.so.1
    #3  0xfffffd7ffc25bb39 in __cxa_throw (obj=<value optimized out>, tinfo=0x1, dest=0x474e5543432b2b00)
        at ../../../../.././libstdc++-v3/libsupc++/eh_throw.cc:78
    #4  0xfffffd7ffaaaa1c8 in shape_datasource::bind () from /usr/local/lib/mapnik/input/shape.input
    #5  0xfffffd7ffaaaadb5 in shape_datasource::shape_datasource () from /usr/local/lib/mapnik/input/shape.input
    #6  0xfffffd7ffaaaae80 in create () from /usr/local/lib/mapnik/input/shape.input
    #7  0xfffffd7ffc4bfa51 in mapnik::datasource_cache::create () from /usr/local/lib/libmapnik.so
    #8  0xfffffd7ffc459d1b in mapnik::map_parser::parse_layer () from /usr/local/lib/libmapnik.so
    #9  0xfffffd7ffc469b00 in mapnik::map_parser::parse_map () from /usr/local/lib/libmapnik.so
    #10 0xfffffd7ffc46b0b7 in mapnik::load_map () from /usr/local/lib/libmapnik.so
    #11 0xfffffd7ffc76b12a in load_map_overloads::non_void_return_type::gen<boost::mpl::vector4<void, mapnik::Map&, std::string const&, bool> >::func_0 () from /usr/local/lib/python2.6/site-packages/mapnik/64/_mapnik.so
    #12 0xfffffd7ffc770950 in boost::python::objects::caller_py_function_impl<boost::python::detail::caller<void (*)(mapnik::Map&, std::string const&), boost::python::default_call_policies, boost::mpl::vector3<void, mapnik::Map&, std::string const&> > >::operator()
        () from /usr/local/lib/python2.6/site-packages/mapnik/64/_mapnik.so
    #13 0xfffffd7ffc2db47c in boost::python::objects::function::call () from /usr/local/lib/libboost_python.so.1.44.0
    #14 0xfffffd7ffc2db730 in boost::detail::function::void_function_ref_invoker0<boost::python::objects::(anonymous namespace)::bind_return, void>::invoke () from /usr/local/lib/libboost_python.so.1.44.0
    #15 0xfffffd7ffc2e3053 in boost::python::detail::exception_handler::operator() () from /usr/local/lib/libboost_python.so.1.44.0
    #16 0xfffffd7ffc76b2e2 in boost::detail::function::function_obj_invoker2<boost::_bi::bind_t<bool, boost::python::detail::translate_exception<mapnik::config_error, void (*)(mapnik::config_error const&)>, boost::_bi::list3<boost::arg<1>, boost::arg<2>, boost::_bi::value<void (*)(mapnik::config_error const&)> > >, bool, boost::python::detail::exception_handler const&, boost::function0<void> const&>::invoke () from /usr/local/lib/python2.6/site-packages/mapnik/64/_mapnik.so
    #17 0xfffffd7ffc2e2e25 in boost::python::handle_exception_impl () from /usr/local/lib/libboost_python.so.1.44.0
    #18 0xfffffd7ffc2d7d50 in function_call () from /usr/local/lib/libboost_python.so.1.44.0
    #19 0xfffffd7ffd309aad in PyObject_Call () from /usr/lib/amd64/libpython2.6.so.1.0
    #20 0xfffffd7ffd3ac1c0 in do_call () from /usr/lib/amd64/libpython2.6.so.1.0
    #21 0xfffffd7ffd3ab672 in call_function () from /usr/lib/amd64/libpython2.6.so.1.0
    #22 0xfffffd7ffd3a810d in PyEval_EvalFrameExReal () from /usr/lib/amd64/libpython2.6.so.1.0
    #23 0xfffffd7ffd3a4d9d in PyEval_EvalFrameEx () from /usr/lib/amd64/libpython2.6.so.1.0
    #24 0xfffffd7ffd3abb95 in fast_function () from /usr/lib/amd64/libpython2.6.so.1.0
    #25 0xfffffd7ffd3ab68a in call_function () from /usr/lib/amd64/libpython2.6.so.1.0
    #26 0xfffffd7ffd3a810d in PyEval_EvalFrameExReal () from /usr/lib/amd64/libpython2.6.so.1.0
    #27 0xfffffd7ffd3a4d9d in PyEval_EvalFrameEx () from /usr/lib/amd64/libpython2.6.so.1.0
    #28 0xfffffd7ffd3abb95 in fast_function () from /usr/lib/amd64/libpython2.6.so.1.0
    #29 0xfffffd7ffd3ab68a in call_function () from /usr/lib/amd64/libpython2.6.so.1.0
```

The most important lines are the top ones:

Basically in this case it clearly indicates that the problem happened in the 'shape.input' or the Shapefile datasource plugin, and specifically the 'bind()' function.

We know the bind function is basically the call to create the datasource, so at creation time one of the values must be wrong. A likely possibility is that the path to a shapefile was wrong and mapnik is throwing a datasource exception to tell us this, but because this exception is not caught we cannot know for sure.

```sh
    #0  0x0000000000053735 in ?? ()
    #1  0xfffffd7fff2ec5d1 in _Unwind_RaiseException_Body () from /usr/lib/amd64/libc.so.1
    #2  0xfffffd7fff2ec855 in _Unwind_RaiseException () from /usr/lib/amd64/libc.so.1
    #3  0xfffffd7ffc25bb39 in __cxa_throw (obj=<value optimized out>, tinfo=0x1, dest=0x474e5543432b2b00)
        at ../../../../.././libstdc++-v3/libsupc++/eh_throw.cc:78
    #4  0xfffffd7ffaaaa1c8 in shape_datasource::bind () from /usr/local/lib/mapnik/input/shape.input
    #5  0xfffffd7ffaaaadb5 in shape_datasource::shape_datasource () from /usr/local/lib/mapnik/input/shape.input
```

To leave gdb do:

    quit


It is a sad thing, but perhaps the only definitive way to figure out what is wrong without being able to fix the ultimate problem is to replace any `throw`s with std::exit like:


```cpp

    Index: plugins/input/shape/shape.cpp
    ===================================================================
    --- plugins/input/shape/shape.cpp       (revision 2441)
    +++ plugins/input/shape/shape.cpp       (working copy)
    @@ -54,8 +54,13 @@
           desc_(*params.get<std::string>("type"), *params.get<std::string>("encoding","utf-8"))
     {
         boost::optional<std::string> file = params.get<std::string>("file");
    -    if (!file) throw datasource_exception("missing <file> parameter");
    -
    +    
    +    if (!file) 
    +    {
    +    std::clog << "missing <file> parameter\n";
    +    std::exit(1);   
    +///throw datasource_exception("missing <file> parameter");
    +}
         boost::optional<std::string> base = params.get<std::string>("base");
         if (base)
             shape_name_ = *base + "/" + *file;
    @@ -76,7 +81,8 @@
     
         if (!boost::filesystem::exists(shape_name_ + ".shp"))
         {
    -        throw datasource_exception("shapefile '" + shape_name_ + ".shp' does not exist");
    +        std::clog << "shapefile '" << shape_name_ << ".shp' does not exist";
    +        std::exit(1);
         }
     
         if (boost::filesystem::is_directory(shape_name_ + ".shp"))
```