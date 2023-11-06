#!/bin/sh

clang -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL vendor/raylib/src/libraylib.a -shared -undefined dynamic_lookup -o build/ui.so src/ui.c

if [ $? -ne 0 ] 
then
    echo "Failure"
else
    python3 main.py
fi