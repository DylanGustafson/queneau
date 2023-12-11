#!/bin/bash

if ! [ -f ./queneau ]; then
	echo Rebuilding executable...
	make
	echo Done! Running now...
fi

a=$(date +%s.%N)
./queneau $1 $2 $3
b=$(date +%s.%N)
t=$(echo "$b-$a" | bc)
echo Total time\: $t seconds
