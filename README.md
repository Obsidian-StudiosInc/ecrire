# ecrire - Text Editor for EFL/Enlightenment
[![License](http://img.shields.io/badge/license-GPLv3-blue.svg?colorB=9977bb&style=plastic)](https://github.com/Obsidian-StudiosInc/ecrire/blob/master/LICENSE)
[![Build Status](https://img.shields.io/travis/Obsidian-StudiosInc/ecrire/master.svg?colorA=9977bb&style=plastic)](https://travis-ci.org/Obsidian-StudiosInc/ecrire)
[![Build Status](https://img.shields.io/shippable/58fa9a131fb3ec0700df16e5/master.svg?colorA=9977bb&style=plastic)](https://app.shippable.com/projects/58fa9a131fb3ec0700df16e5/)

This is a mirror of the official
[ecrire repo](https://git.enlightenment.org/apps/ecrire.git/)

Work is being done in the wltjr branch. Intended to be merged back into 
upstream, or forked worse case.

## Original Readme

This is intended to be a text editor.

It's currently mostly just a PoC, but I hope it'll grow to be more than that.

HOWTO Build
$ mkdir build
$ cd build
$ cmake ..
$ make && sudo make install

If you're trying to build in a machine where you have efl 1.7.5 libraries installed, 
you must run cmake like this:
$ CMAKE_PREFIX_PATH=/path/to/ecrire/cmake/Modules/legacy cmake /path/to/ecrire

If you are trying to build with efl 1.8 and having issues, you should try:
$ CMAKE_PREFIX_PATH=/usr/local/share/cmake/Modules/ cmake /path/to/ecrire
