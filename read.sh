#!/bin/bash
> out
dd if=/dev/yes of=out count=$1
echo
cat out
echo
