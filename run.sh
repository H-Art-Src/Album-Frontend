#!/bin/sh
cc -fPIC -shared -o build/ui.so src/ui.c -lraylib

if [ $? -ne 0 ] 
then
    echo "Failure"
else
    python3 main.py
fi