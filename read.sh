#!/bin/bash
if [ -z "$1" ]
then
	dev="yes"
else
	dev="$1"	
fi

if [ -z "$2" ]
then
	num="12"
else
	num="$2"	
fi

> out
dd if=/dev/$dev of=out count=$num
echo
cat out
echo
