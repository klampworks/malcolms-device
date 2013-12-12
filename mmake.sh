#!/bin/bash
make -C ../linux-3.2.51/ M=/home/debian/malcolms-device/ modules CFLAGS="-E -g -O2"
rmmod yes 
insmod yes.ko
