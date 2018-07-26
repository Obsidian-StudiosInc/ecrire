#!/bin/bash

xvfb-run
src/bin/ecrire &
sleep 5
killall -q ecrire
