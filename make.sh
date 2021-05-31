#!/bin/sh

cc -O2 usbisp.c spmp_usbisp.c $(pkg-config --cflags --libs libusb-1.0) -o usbisp
cc -O2 usbreg.c spmp_usbisp.c $(pkg-config --cflags --libs libusb-1.0) -o usbreg
