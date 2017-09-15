# ecrire - Text Editor for EFL/Enlightenment
[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/Obsidian-StudiosInc/ecrire/blob/master/LICENSE)
[![Build Status](https://img.shields.io/travis/Obsidian-StudiosInc/ecrire/master.svg?colorA=9977bb&style=plastic)](https://travis-ci.org/Obsidian-StudiosInc/ecrire)
[![Build Status](https://img.shields.io/shippable/58fa9a131fb3ec0700df16e5/master.svg?colorA=9977bb&style=plastic)](https://app.shippable.com/projects/58fa9a131fb3ec0700df16e5/)
[![Code Quality](https://img.shields.io/coverity/scan/12512.svg?colorA=9977bb&style=plastic)](https://scan.coverity.com/projects/obsidian-studiosinc-ecrire)

This is a fork and current development version Ecrire, a EFL based text 
editor for Enlightenment and eventually Tizen as well. Or any device 
that has EFL.

## Known Issues
- Crash when opening large files
[issue #4](https://github.com/Obsidian-StudiosInc/ecrire/issues/4)
[elm code task #T5497](https://phab.enlightenment.org/T5497)
- External 3rd button copy/paste does not function [elm code task #T5520](https://phab.enlightenment.org/T5520)
- Line/Word wrap not possible till available in elm_code [elm code task #5908](https://phab.enlightenment.org/T5908)

## About
Ecrire is a basic text editor written in EFL for the Enlightenment 
desktop environment and long term also Tizen. It is intended to be a 
native EFL alternative to gedit (GTK/Gnome), kwrite (KDE/Plasma), and 
similar basic text editors. With the exception that ecrire should be 
usable on desktop as well as mobile devices.

Ecrire has been modified to use elementary code widget instead of entry. 
There are various issues as a result of this change, and some 
pre-existing issues. Some pre-existing issues have been fixed with the 
switch and others new issues have been created.

Issues relating to syntax highlighting are being tracked in the 
[Elm Code Syntax Support chart](https://phab.enlightenment.org/w/elm_code/syntax_support/). 
Please do not open tasks on syntax highlighting related issues here on github. 
Please open tasks on 
[Enlightenment's Phabricator](https://phab.enlightenment.org/) 
for elm code. That is the best place, since those issues will have to be 
addressed upstream in EFL itself.

Work is ongoing to get Ecrire into shape for basic daily use. Lots of 
features will be coming after the most common are implemented, including 
effects/eye candy.

Contributions are welcome!

## Build
ecrire uses the cmake build system, little chance of switching to meson. 
The build system will generate deb and rpm's for installation, using 
either dist or package targets. Dist target will package sources in 
addition to binaries. Package will only generate deb and rpm's.

## Autotools
The default build of cmake uses autotools

```
cmake .
make package
```

## Ninja
To use ninja for faster build instead of autotools
```
cmake -G Ninja
ninja package
```

Then for either autotools or ninja, you can install via the resulting 
.deb or .rpm generated in the dist directory.
