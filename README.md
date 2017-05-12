# ecrire - Text Editor for EFL/Enlightenment
[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/Obsidian-StudiosInc/ecrire/blob/master/LICENSE)
[![Build Status](https://img.shields.io/travis/Obsidian-StudiosInc/ecrire/wltjr.svg?colorA=9977bb&style=plastic)](https://travis-ci.org/Obsidian-StudiosInc/ecrire)
[![Build Status](https://img.shields.io/shippable/58fa9a131fb3ec0700df16e5/wltjr.svg?colorA=9977bb&style=plastic)](https://app.shippable.com/projects/58fa9a131fb3ec0700df16e5/)
[![Code Quality](https://img.shields.io/coverity/scan/12512.svg?colorA=9977bb&style=plastic)](https://scan.coverity.com/projects/obsidian-studiosinc-ecrire)

This is a mirror of the official
[ecrire repo](https://git.enlightenment.org/apps/ecrire.git/)

Work was being done in the wltjr branch. It has been merged into master 
branch and further work will be done there. This repository should be 
sync'd with the official Enlightenment one within the next month.

## Known Issues
- Search Previous button goes same direction as next, one way vs two way search
- Crash opening files containing code, other than c source and header files
- Wordwrap setting does not function till next release of EFL > 1.19.0
- Issues running under Wayland [Issue #2](https://github.com/Obsidian-StudiosInc/ecrire/issues/2)

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
To build for usage run cmake, and then make. The build system will 
generate deb and rpm's for installation, using either dist or package 
targets. Dist target will package sources in addition to binaries. 
Package will only generate deb and rpm's.

```
cmake .
make package
```

Then you can install via the resulting dep or rpm generated in the dist 
directory.
