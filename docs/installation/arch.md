# Installing Mapnik on Arch Linux

Mapnik is available as a community package. To install it, just run:

    # pacman -Sy mapnik

## Mapnik from Git

If for any reason you want to build Mapnik from source, a package description is available in the Arch User Repository for easy building with ABS: [mapnik-git](https://aur.archlinux.org/packages.php?ID=53270) (this will compile the latest development version from the Git master branch).

If you use [Yaourt](https://wiki.archlinux.org/index.php/Yaourt) or a similar [AUR helper](https://wiki.archlinux.org/index.php/AUR_Helpers), just install the 'mapnik-git' package:

    $ yaourt -S mapnik-git

This will handle downloading, building and installing Mapnik with dependencies from the 'extra' and 'community' repositories.
