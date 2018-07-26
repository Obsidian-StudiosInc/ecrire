#!/bin/bash

xvfb-run src/bin/ecrire README.md &
sleep 5
killall -q ecrire
