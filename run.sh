#!/bin/sh
mkdir build
mkdir build/coverImages
cc -fPIC -shared -o build/ui.so src/ui.c -s
if [ $? -ne 0 ]
then
    echo "Failure"
else
    cp src/main.py build/main.py
    cd build
    python3 main.py
fi