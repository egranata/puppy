#!/bin/bash

CURDIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null && pwd )"
i686-elf-g++ -specs="$CURDIR/../out/mnt/libs/gcc.specs" $@
