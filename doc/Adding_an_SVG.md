# Adding an SVG to the resources bundle

On a machine with Perl, run:

```sh
perl scripts/misc/perl/addSVG.pl (filename) (idname) (idnum)
```

For example:

```sh
perl ./scripts/misc/addSVG.pl assets/SurgeClassic/exported/bmp00307.svg IDB_MSEG_CONTROLPOINT 307
```

Well-named SVGs in `assets/SurgeClassic/exported` are compiled into the bundle. To add
one:

1. Add its ID to `IDB_` in `resource.h`.
2. Add an `addEntry` in `SurgeBitmaps.cpp`. This completes the Mac and Linux setup.
3. Add `IDB_BLAH_SVG_DATA` to `svgresources.rc`.
4. Add `IDB_MENU_IN_SLIDER_SCALE_BLAH` defines to `scalableresource.h`.
5. Add the forward name at the top of `src/common/gui/SkinImageMaps.h`.
6. Add the ID to the set at the bottom of `src/common/gui/SkinImageMaps.h`.
