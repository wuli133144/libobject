#!/bin/sh

for i in autoconf; do
    echo "$i"
    $i
    if [ $? -ne 0 ]; then
	echo "Error $? in $i"
	exit 1
    fi
done

echo "./configure --enable-autogen $@"
if [ "$1"x = "aarch64"x ]; then
    	./configure --enable-autogen --build=x86_64-linux-gnu --host=aarch64-linux-gnu --target=aarch64-linux-gnu $@
else
	./configure --enable-autogen
fi
if [ $? -ne 0 ]; then
    echo "Error $? in ./configure"
    exit 1
fi
