#!/bin/sh
cc -fPIC -shared -o build/ui.so src/ui.c -s -static -Os -std=c99 -Wall -Ivendor/raylib/src -Iexternal -DPLATFORM_DESKTOP -lraylib -lopengl32 -lgdi32 -lwinmm

if [ $? -ne 0 ] 
then
    echo "Failure"
else
    python3 main.py
fi