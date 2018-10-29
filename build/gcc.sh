#!/bin/sh

i686-elf-gcc -specs=$(dirname $0)/newlib.specs $@
