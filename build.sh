#!/bin/bash

mkdir -p build
cp *.c wanddef.h Makefile build
cd build

# Script per modificar la línia 125 de wanddef.h
# This shuld be 's/struct  paramstr {(?s)(.*)} param;/extern struct  paramstr {$1} param;/m'
# but I canot make sed to work
sed -i 's/struct  paramstr/extern struct  paramstr/' wanddef.h

make wander -C .
