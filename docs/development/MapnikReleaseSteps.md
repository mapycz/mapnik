# Steps for Mapnik Releases
    
### Prepare
    
- Ensure the [CHANGELOG](https://github.com/mapnik/mapnik/blob/master/CHANGELOG.md) is up to date.
- Announce release plans to [group list](http://groups.google.com/group/mapnik)
- Ensure [milestone](https://github.com/mapnik/mapnik/milestones) is closed out

### Testing

- Ensure all tests pass (`make test`)
- Test Mapnik with `INPUT_PLUGINS=''` and ensure tests pass for what functionality is available
- Test with `./configure WARNING_CXXFLAGS="-Wextra"` to ensure all problematic warnings are solved or triaged.

### Bundled fonts and scons

OPTIONAL: Consider updating Scons-local to [latest release](http://www.scons.org/download.php): The last SCons update was 2.5.0.
    
```sh
wget http://prdownloads.sourceforge.net/scons/scons-local-2.5.0.zip
rm -rf scons
unzip -o scons-local-*.zip -d scons/
rm scons-local-*.zip
```

OPTIONAL: Consider updating DeJaVu Fonts: The [last version](http://dejavu-fonts.org/wiki/Download) updated was 2.37

```sh
cd fonts
git rm -r dejavu-fonts-ttf-
wget http://sourceforge.net/projects/dejavu/files/dejavu/2.37/dejavu-fonts-ttf-2.37.tar.bz2
tar xvf dejavu-fonts-ttf-2.37.tar.bz2
git add dejavu-fonts-ttf-2.37
```
Make sure to update font paths in `demo/c++/rundemo.cpp` 

Check for new [unifont release](http://unifoundry.com/unifont.html)

### Release candidate

OPTIONAL: If it has been > 3 months since the last official release, consider first promoting a release candidate first:

```sh
git tag -a v3.0.0-rc1 -m 'Release Candidate 1 for Mapnik v3.0.0'
git push --tags
```

### Pre-tag updates

  * Update version number in [version.hpp](https://github.com/mapnik/mapnik/blob/master/include/mapnik/version.hpp)

```
make uninstall && ./configure && make
MAPNIK_VERSION=`./utils/mapnik-config/mapnik-config --version`
git commit -a -m "setting up for mapnik v${MAPNIK_VERSION} release [skip ci]" 
git push
```

  * Update CHANGELOG with the git hash of latest commit using the output of:

```
git describe # take hash after 'g'
```

  * Then, push change:

```
git commit -a -m "update CHANGELOG for mapnik v${MAPNIK_VERSION} release [skip ci]"
git push
```

### Tagging

We use [annotated tags](http://stackoverflow.com/questions/4971746/why-should-i-care-about-lightweight-vs-annotated-tags/4971817#4971817) below instead of lightweight tags

```sh
MAPNIK_VERSION=`mapnik-config --version`
git tag --annotate "v${MAPNIK_VERSION}" -m "tagging v${MAPNIK_VERSION}"
git push --tags
```

* Create and upload clean tarball:

Before running this you'll need:

 - Your mapnik clone needs to be on a tag checkout (either master after tagging or with a tag checked out explicitly) such that `git describe` gives a clean tag (no trailing `-gGITSHA`).
 - An environment variable called `GITHUB_TOKEN_MAPNIK_PUBLIC_REPO` set with a token with `public_repo` scope.
 - The submodules up to date

```sh
make release
```

* Test the uploaded tarball:

```sh
make test-release
```

If the `make test-release` build works then go to https://github.com/mapnik/mapnik/releases, find the latest draft release created by `make release` and publish it publically.

### Post tag updates

* If there are new styling or datasource options, create a new `mapnik-reference` entry for the release: https://github.com/mapnik/mapnik-reference

* Update master branches entries in [CHANGELOG](https://github.com/mapnik/mapnik/blob/master/CHANGELOG.md) from the new release (if relevant, e.g. if you are tagging and releasing a stable release not from the master branch).

If this was a major release and a stable series is likely, now branch it, for example a `2.1.0` release would warrant an immediate `2.1.x` branch for a stable series of bugfix releases.

```
cd ${MAPNIK_SOURCES}
git branch 2.1.x
git checkout 2.1.x
git push origin 2.1.x
git checkout master
```

Now bump versions again:

   * edit [version.hpp](https://github.com/mapnik/mapnik/blob/master/include/mapnik/version.hpp) again, incrementing version

```
make uninstall && ./configure && make install
MAPNIK_VERSION=`mapnik-config --version`
git ci include/mapnik/version.hpp -m "now working on mapnik v${MAPNIK_VERSION} [skip ci]"
git push
```

_Now also repeat the above for any stable branches created._

Finally, create new github milestones for the newly created future release #s.

And create new launchpad PPA for the target release(s) and series at https://launchpad.net/~mapnik, then add these release PPA's to the list that gets build nightly: https://github.com/mapnik/mapnik-packaging/blob/master/debian-nightlies/nightly-build.sh#L22-40

### Update Mapnik.org

* Update the [download page](http://mapnik.org/download/)(pages/downloads.html)
* Write new blog post with updated release links and links to changelog
* Push python api docs and update docs/index.markdown
    
### Packaging

1) Submit pull request for homebrew formula

 - Fork homebrew
 - Edit https://github.com/mxcl/homebrew/blob/master/Library/Formula/mapnik.rb
 - Change the version
 - Run `brew install mapnik`, will fail on `sha256` check
 - grab expected `sha256` from error message, edit `mapnik.rb`
 - test building `brew install mapnik`
 - submit pull request - learning from older ones like https://github.com/Homebrew/homebrew/pull/41474
   
2) Package binaries for Ubuntu Linux (PPA)

 - Scripts are at https://github.com/mapnik/debian
 - Create a new version by copying `master` scripts (or appropriate dir)
 - Add an entry for the new version to https://github.com/mapnik/debian/blob/master/nightly-build.sh
 - The `nighly-build.sh` is run on a cron by robert.coup@koordinates.com (@rcoup) - TODO - should we move this to travis?

3) Upload Mac/Win binary packages to the s3 bucket: <http://mapnik.s3.amazonaws.com/dist/>

TODO - currently do not have bandwidth or set process for this. For Mac: In the past @springmeyer created mac easy installer but just recommending homebrew is better now (since they support binaries/bottles). For Windows: https://github.com/mapbox/windows-builds is used to create SDK's but we've not yet formalized document how these can be used (those they are viable).

### Wiki Post-Release

* Add https://github.com/mapnik/mapnik/wiki/Release${VERSION} (needed by ubuntu packages)
* Update [[Mapnik-Installation]], [MacInstallation](MacInstallation), [LinuxInstallation](https://github.com/mapnik/mapnik/wiki/LinuxInstallation) and [WindowsInstallation](WindowsInstallation) links
* Create a release page from the relevant section of CHANGELOG like this page [Release0.7.1](Release0.7.1)
* Update [MapnikReleases](MapnikReleases), a starting page for users to learn about Mapnik development
    
### Announce
    
* Mapnik users list
* Mapnik news twitter account: http://twitter.com/mapnikproject
* OpenStreetMap : [OSM-Dev]
* Notify Packagers for Linux distros ([PackageBuilding](PackageBuilding))
* Notify Packagers for OSGEO4w: http://norbit.de/