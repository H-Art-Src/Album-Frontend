#!/bin/sh
cc -fPIC -shared -o ui.so ui.c -lraylib

if [ $? -ne 0 ] 
then
    echo "Failure"
else
    python3 main.py
fi