# ecrire - Text Editor for EFL/Enlightenment
[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/Obsidian-StudiosInc/ecrire/blob/master/LICENSE)
[![Build Status](https://img.shields.io/travis/Obsidian-StudiosInc/ecrire/wltjr.svg?colorA=9977bb&style=plastic)](https://travis-ci.org/Obsidian-StudiosInc/ecrire)
[![Build Status](https://img.shields.io/shippable/58fa9a131fb3ec0700df16e5/wltjr.svg?colorA=9977bb&style=plastic)](https://app.shippable.com/projects/58fa9a131fb3ec0700df16e5/)

This is a mirror of the official
[ecrire repo](https://git.enlightenment.org/apps/ecrire.git/)

Work is being done in the wltjr branch. Intended to be merged back into 
upstream, or forked worse case.

## About
Ecrire is a basic text editor written in EFL for the Enlightenment 
desktop environment. It is intended to be a native EFL alternative to 
gedit (GTK/Gnome), kwrite (KDE/Plasma), and similar basic text editors.

Ecrire has been modified to use elementary code widget instead of entry. 
There are various issues as a result of this change, and some 
pre-existing issues. Some pre-existing issues have been fixed with the 
switch and others new issues have been created.

Work is ongoing to get Ecrire into shape for basic daily use. 
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
