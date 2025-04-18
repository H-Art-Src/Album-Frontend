#!/bin/sh
mkdir build
mkdir build/coverImages
cc -fPIC -shared -o build/ui.so src/ui.c -s -static -Os -std=c99 -Wall -Ivendor/raylib/src -Iexternal -DPLATFORM_DESKTOP -lraylib -lopengl32 -lgdi32 -lwinmm

if [ $? -ne 0 ] 
then
    echo "Failure"
else
    cp src/main.py build/main.py
    cd build
    python3 main.py
fi