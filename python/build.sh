#!/bin/sh

python setup.py build
python3 setup.py build

if [ "x$1" == "x-i" ]; then
    sudo python setup.py install
    sudo python3 setup.py install
fi
