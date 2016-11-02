<!-- Name: OpenSolarisInstallation -->
<!-- Version: 44 -->
<!-- Last-Modified: 2010/12/08 14:44:45 -->
<!-- Author: springmeyer -->


# Installing Mapnik and Dependencies on Open Solaris

This guide will describe getting Mapnik and it's dependencies running on the latest Open Solaris release: *2009.06*.

This is a *non-trivial* exercise. It is highly recommended that you consider using Linux unless Open Solaris is a must. This document was written in late 2010 - your mileage may vary with later versions of Open Solaris. Basically package names may change and other workarounds below (hopefully) will become unneeded.

In the future hopefully the latest versions of the [GNU compiler (gcc)](http://gcc.gnu.org/) will become better supported on Open Solaris, or better packaging options may be available with  [OpenIndiana](http://openindiana.org/) or [Nexenta](http://www.nexenta.org/).

Basically the challenge here is boost and old, buggy compilers that sun (or sun packages) provide. If you you simply forge ahead compiling boost with suncc or the g++ provided by sun on open solaris you'd hit boost regex, boost python, and boost thread compile errors. You'll likely see problems such as those detailed at the [Open Solaris Troubleshooting page](OpenSolarisInstallation_TroubleShooting).

If you are finding this guide and do not need to install the boost libraries then this approach is overkill and likely the wrong approach. Most geospatial packages (like geos, proj4, postgis) should compile just fine on open solaris with the existing versions of gcc available. Instead this guide uses a custom compiled gcc44 which makes compiling boost possible, and works great to compile all other packages. The catch is that this approach requires much more care and attention (and some would likely call it insane to built up a whole geostack using a custom compiler not officially supported on the os).

If anyone does forge ahead using the old tools available from sun and can compile Mapnik trunk and run the python nosetest successfully, then the approach of using gcc44 below may be able to be avoided. If you are this daft, then please update this page with what you've learned.

## Testing Details

 * Tested running VirtualBox on OSX 10.6 (64 bit)
 * Tested using the latest OSOL iso release:

```sh
    $ cat /etc/release
       OpenSolaris 2009.06 snv_111b X86
       Assembled 07 May 2009
```

## Mapnik Details

You will need at least:

 * Mapnik >= 2.x

For 64 bit builds you need to apply patches from [#675](https://github.com/mapnik/mapnik/issues/675) and [#676](https://github.com/mapnik/mapnik/issues/676)

## Build Details

This guide will rely heavily on existing tools for package management on Open Solaris

 * The 'pkg' tool supplied by Sun
 * The community 'pkgtool/pkgbuild' tools
 * The excellent WikiMedia Toolserver `spec` files for use with `pkgtool`


## OpenSolaris Gotchas

 * Don't use `sudo` use `pfexec` (alias sudo="pfexec")
 * Vim keystrokes are broken when logging in via ssh from a mac. But typing `:set nocp` fixes things.
 * Get info on CPUS: `prtdiag -v`
 * `-G` is the solaris compiler flag for creating a shared library, but recently gcc versions need `-shared`
 * If the `-shared` flag is missing then your libs will not be properly 'PIC" (position independent code) and this can be diagnosed with:
```
    elfdump -d /path/to/mylibrary.so | grep TEX # if PIC this should return nothing, if not PIC it will return a few rows
```
 * Library runtime linking works differently on solaris than linux.
  * It is more like mac osx, where libraries hardcode runtime search paths at build time
  * Except that Solaris needs a special flag to get this to work: `-R/usr/local/lib`
  * To figure out where a library looks for depedents at runtime do:

    dump -Lv /usr/local/lib/mylibrary.so

## Step 1: Updating Open Solaris

Optional: If running VirtualBox, install the guest additions then reboot

```sh
    pfexec reboot
```

First we need to update the package manager

```sh
    pfexec pkg install SUNWipkg
```

Next, we update to the latest Open Solaris Development environment

*Note:* If we update the base os image after setting the publisher to the "dev", the gnome/x11 user interface will break and we will only have a command line interface remaining. But, we want to update to dev (and the below guide depends upon it) because we want the ease of getting various packages from dev (via`pkg`) that otherwise we would need to install from source with `ts-specs/pkgtool`. The error upon rebooting that you will see (indicating the loss of X11) is `inetd[6270[: Failed to update state of instance svc: /application/x11/xfs:default in repository`. So, if you don't want to update to the dev environment as you don't want to break the GUI, don't set the publisher before running image-update (and know that certain commands in this guide will fail because dependencies are not going to be found).

```sh
    pfexec pkg set-publisher -O http://pkg.opensolaris.org/dev opensolaris.org
    pfexec pkg refresh --full
    # update the package manager if you did not already (otherwise next command will fail)
    pfexec pkg install SUNWipkg
    # optional: test upgrade of system
    pfexec pkg image-update -nv
    # do actual upgrade (note: upgrades not possible on ec2)
    pfexec pkg image-update
    pfexec reboot # then boot into new 'opensolaris-1'
```

When you reboot you should see the new version reflected in `/etc/release`:

```sh
    cat /etc/release
    # should get:
                           OpenSolaris Development snv_134 X86
               Copyright 2010 Sun Microsystems, Inc.  All Rights Reserved.
                            Use is subject to license terms.
                                 Assembled 01 March 2010
```

*Note:* This is a new boot environment, see the [solaris docs](http://developers.sun.com/developer/technicalArticles/opensolaris/bootenvironments/index.html) for more info.

## Step 2: Setting up Sun Studio and Package Installation Tools

Install Sun Studio and a few other sun provided base development tools you will need to get started:

```sh
    # install dev tools from sun needed specifically for mapnik build
    pfexec pkg install SUNWsvn SUNWpython26 SUNWgmake SUNWgcc developer/sunstudio12u1
    
    # install other dev tools you'll likely need down the road
    pfexec pkg install gcc-dev SUNWsshd SUNWgnu-readline SUNWcurl SUNWapr13 SUNWapu13 gd
```

 * NOTE: python26 is available only due to the pkg-update to `dev` which brings us to `snv_134`. If you can and want to compile your own python version, then updating to dev is less critical. 

 * CRITICAL: boost_python and mapnik, compiled with gcc44 can run just fine, but if used with the sun proved python26 (which will be linked to a different version of libgcc) then exception handling will likely be broken: http://mail.python.org/pipermail/cplusplus-sig/2010-December/015822.html. At this time this guide uses this approach and their is not currently a clean solution for this problem (short of compiling python from source and all the issues that may arise from that).

## Step 3: Setting up WikiMedia Toolserver's 'ts-specs'

The WikiMedia project runs Solaris for many of its systems and maintains an excellent rpm-like set of package specs which can be used to built up a robust development and production environment.

The CATCH with using these build files is that they are written for Solaris not Open Solaris - so *many* hardcoded paths are correct but some are not and cause packages to break unless you are willing to dig deeply to figure out what may be going wrong. So, we do not recommend using the *ts-specs* for anything other that getting gcc44 built, but in the future the *ts-specs* project may have a lot of promise as an awesome way to build up a geo stack.

So, overally we choose this route because using suncc (Sun's compiler) or the gcc version available in the standard open solaris packages (4.3.x) will lead to hard and lonely dead-ends compiling boost. Boost is an excellent set of libraries but pushes compilers and exposes their bugs. So, basically we need gcc/g++ 4.4 for building boost, and we can get it built, as well as boost (if we choose to, but in this guide we do not), using spec files from WikiMedia Toolservers 'ts-specs'.

Download the 'ts-spec' repository:

```sh
    svn co http://svn.toolserver.org/svnroot/toolserver/trunk/ts-specs -r 568 
    cd ts-specs
    # confirm that packages should be downloaded locally
    echo download >~/.pkgtoolrc
```

 *NOTE: we fetch a specific revision above just for caution and so that the below patch will be sure to apply cleanly. You can update to the latest *ts-specs* if you wish.

Now we need the pkgtool program to use these spec files. Ultimately we'll install this tool using ts-specs, but first we fetch it and compile ourselves:

```sh
    # as the root user build latest pkgbuild
    # http://pkgbuild.sourceforge.net/pkgtool.html
    wget http://prdownloads.sourceforge.net/pkgbuild/pkgbuild-1.3.103.tar.bz2
    tar xvf pkgbuild-1.3.103.tar.bz2
    cd pkgbuild-1.3.103
    ./configure --prefix=/opt/pkgbuild # use a custom prefix so it is easy to uninstall later
    make
    pfexec make install
```

If you do not already have a non-root user you are using, then set one up now to run the ts-spec installs (pkgtool insists!).

Optional: If you don't already have a non-root user with admin access do:

```sh
    groupadd dev # or your preferable group name
    export USERNAME=<user> # your new username
    useradd -g dev -d /export/home/$USERNAME -m -s /bin/bash -c "$USERNAME" $USERNAME
    passwd $USERNAME
    export HOME=/export/home/$USERNAME
    cd $HOME
    mkdir src
    export src=$HOME/src
    export PATH=/opt/ts/bin/:$PATH
    # give the user rights
    # http://developers.sun.com/developer/technicalArticles/opensolaris/pfexec.html
    usermod -P'File System Management' $USERNAME
    usermod -P'Software Installation' $USERNAME
    # giving sudo/admin privs
    usermod -P'Primary Administrator' $USERNAME
```

Once done, log out and back in as this new user

Then, finally install pkgtool via ts-specs and uninstall the source compiled version.

```sh
    cd ts-specs # move into ts-specs directory that you download above
    # as non-root user...
    /opt/pkgbuild/bin/pkgtool build sysutils/TSpkgbuild
    # then remove /opt/pkgbuild
    pfexec rm -rf /opt/pkgbuild
```

The expected output from pkgtool is:

```sh
    INFO: Copying %use'd or %include'd spec files to SPECS directory
    INFO: Processing spec files
    INFO: Finding sources
    INFO: Downloading source http://mesh.dl.sourceforge.net/sourceforge/pkgbuild/pkgbuild-1.3.98.4.tar.bz2
    INFO: Running pkgbuild -ba [...] TSpkgbuild.spec (TSpkgbuild)
    INFO: TSpkgbuild PASSED
    INFO: Installing TSpkgbuild
    libnotify-Message: Unable to get session bus: /usr/bin/dbus-launch terminated abnormally with the following error: Autolaunch error: X11 initialization failed.
    
    Summary:
    
                             package |      status | details
    ---------------------------------+-------------+-------------------------------
                          TSpkgbuild |      PASSED | 

```

Later on when using pkgtool you will likely see these harmless errors:

```
    pkgparam: ERROR: unable to locate parameter information for "SUNWcar"
    pkgparam: ERROR: unable to locate parameter information for "SUNWkvm"
```

Now set up the path to the ts-specs install prefix:

```sh
    export PATH=/opt/ts/bin/:$PATH # or put this in your bashrc...
```

If you are new to pkgtool check out all the default settings like:

```
    /opt/ts/bin/pkgtool --dumprc
```

## Step 4: Patching ts-specs

We are almost ready to start cranking away on installs of the latest dev tools via ts-specs.

One *critical* issue is that gnu tools have a circular dependency on *textinfo*. Luckily ts-specs can build up a base set of tools without using textinfo.

We just need to patch our ts-specs directory to disable textinfo and bypass this circular dependency. 

```sh
    cd ts-specs
    wget http://http://github.com/mapnik/mapnik/wiki/raw-attachment/wiki/OpenSolarisInstallation/ts-spec1-compile-gcc44.patch
    patch -p0 < ts-spec1-compile-gcc44.patch
```

## Step 5: Installing Development Environment

Now we are ready for our base installs via ts-specs.

Here are the commands that work to install gcc4.4:

```sh
    # build dependencies for gcc4.4
    pkgtool build devel/TSm4
    pkgtool build libs/TSgmp
    pkgtool build libs/TSmpfr
    pkgtool build devel/TSbinutils
    
    # finally we'll build gcc4.4
    pkgtool build devel/TSgcc44
    # remove sun gcc - this may fail, but should not be a big deal if it does
    pfexec pkg uninstall SUNWgcc
    # add new gcc on path
    export PATH=/opt/ts/gcc/4.4/bin/:$PATH
    
    # build gmake from ts-spec
    pkgtool build devel/TSgmake
    
    # now remove sun gmake
    pfexec pkg uninstall SUNWgmake
    
    # python26 is already available, but grab a few extra python tools
    pfexec pkg install SUNWpython26-setuptools SUNWpython26-lxml
```

## Step 6: Installing Core Mapnik Dependencies

We're going to do this from source, rather than use ts-specs just to make any debugging much easier (things can go wrong with ts-specs on opensolaris because it has only really been tested on solaris). But in the future I may revise this to try to build more (or all) Mapnik core dependencies with ts-specs.

### 6a: 32 bit - Postgres 8.4 from source

Go to -> [32bit](OpenSolarisInstallation_32bit)

This was my first approach, and it worked great. But, I needed osm2pgsql to run 64 bit (to handle osm planet imports), and postgres to run 64 bit for better performance. So the step below is the current effort.

### 6b: 64 bit - Postgres 8.3 from sun

Go to -> [64bit](OpenSolarisInstallation_32bit)

This approach attempts to get the whole stack running 64 bit, and 64 bit postgres and python are used from sun. Integrating with sun's postgres and python is really hairy, but postgres8.3 was desirable because it is known by the osm community to be faster than 8.4 for applying diffs with osm2pgsql and python from sun was desirable because it is highly customized to be able to run both 32 bit and 64 bit. However the *catch* with this approach is that mapnik's exceptions are broken (http://mail.python.org/pipermail/cplusplus-sig/2010-December/015822.html).



## Step 7: Installing Mapnik

Install Mapnik 0.7.2 or trunk (Mapnik2)

For 64 bit builds you may need to apply patches from [#675](https://github.com/mapnik/mapnik/issues/675) and [#676](https://github.com/mapnik/mapnik/issues/676)

Mapnik 0.7.2:

32bit:

```sh
    cd $SRC
    git clone git://github.com/mapnik/mapnik.git mapnik-0.7.3
    git checkout v0.7.3
    cd mapnik-0.7.2
    export LANG="C"
    export LC_ALL="C"
    python scons/scons.py configure \
      CXX="/opt/ts/gcc/4.4/bin/g++" \
      OPTIMIZATION=3 \
      INPUT_PLUGINS=shape,gdal,postgis \
      PG_CONFIG=/usr/local/pgsql/bin/pg_config \
      GDAL_CONFIG=/usr/local/bin/gdal-config \
      PYTHON_PREFIX=/usr/local/
    
    python scons/scons.py
    pfexec python scons/scons.py install
```

64 bit:

```
    python scons/scons.py configure \
      CXX="/opt/ts/gcc/4.4/bin/g++ -m64 -R/opt/ts/gcc/4.4/lib/amd64/ -R/usr/local/lib -R/usr/postgres/8.3/lib/amd64" \
      CC="/opt/ts/gcc/4.4/bin/gcc -m64 -R/opt/ts/gcc/4.4/lib/amd64/ -R/usr/local/lib -R/usr/postgres/8.3/lib/amd64" \
      OPTIMIZATION=3 \
      INPUT_PLUGINS=shape,gdal,postgis \
      PG_CONFIG=/usr/postgres/8.3/bin/amd64/pg_config \
      GDAL_CONFIG=/usr/local/bin/gdal-config \
      PYTHON_PREFIX=/usr/local/ \
      PYTHON=/usr/bin/amd64/python \
      DEMO=True \
```

NOTE: in 64 bit mode python exceptions will be broken (you will get segfaults instead) due to incompatible libgcc versions linked to by mapnik (/opt/ts/gcc/4/4/lib/ammd64/libgcc_s.so) vs python (/usr/sfw/lib/amd64/libgcc_s.so). Only know solution at this point is to paste this command before running python process that imports mapnik:

```
    LD_PRELOAD=/usr/sfw/lib/amd64/libgcc_s.so /usr/bin/amd64/python
```

Mapnik2:

32 bit:

```sh
    # mapnik trunk
    cd $SRC
    git clone git://github.com/mapnik/mapnik.git mapnik-trunk
    cd mapnik-trunk
    export LANG="C"
    export LC_ALL="C"
    python scons/scons.py configure \
      CXX=/opt/ts/gcc/4.4/bin/g++ \
      OPTIMIZATION=3 \
      INPUT_PLUGINS=shape,gdal,postgis \
      PG_CONFIG=/usr/local/pgsql/bin/pg_config \
      GDAL_CONFIG=/usr/local/bin/gdal-config \
      PYTHON_PREFIX=/usr/local/
    
    python scons/scons.py
    pfexec python scons/scons.py install
```

64bit:

    see command for 0.7.2 above - Mapnik2 should work just as well, but I've not tested 64 bit

## Step 8: Testing Mapnik

Run the nosetests:

```sh
    cd $SRC
    wget http://somethingaboutorange.com/mrl/projects/nose/nose-0.11.2.tar.gz
    tar xvf nose-0.11.2.tar.gz
    cd nose-0.11.2
    pfexec python2.6 setup.py install
    
    # now, within mapnik source directory
    python tests/run_test.py
    # with trunk you should get:
    FAILED (TODO=9, errors=8, failures=4)
    # with Mapnik 0.7.1 you should get no errors or failures
```

Test actual rendering (this assumes you've got an osm db setup up with data using osm2pgsql):

```sh
    # install nik2img
    svn co http://mapnik-utils.googlecode.com/svn/trunk/nik2img/ nik2img-svn
    cd nik2img-svn
    pfexec python setup.py install
    # grab osm stylesheets
    svn co http://svn.openstreetmap.org/applications/rendering/mapnik/ osm-mapnik
    cd osm-mapnik
    ./generate_xml.py --dbname osm --accept-none
```

If using Mapnik2:

```
    # if using mapnik trunk upgrade xml into new copy:
    upgrade_map_xml.py osm.xml osm2.xml
    # test rendering
    nik2img.py osm2.xml image.png --mapnik-version 2
```

Otherwise:

```
    nik2img.py osm.xml image.png
```