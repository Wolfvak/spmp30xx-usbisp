#!/bin/sh

cc -O2 main.c spmp_usbisp.c $(pkg-config --cflags --libs libusb-1.0) -o usbisp
