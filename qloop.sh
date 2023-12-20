#!/bin/bash
chunk=1000000
zeros="000000000"
exp=${#zeros}

dir_execute="/Users/gustafsondy/queneau"
dir_storage="/UsersGbl/gustafsondy/queneau"
x="$dir_execute/queneau"
tfile="$dir_storage/totals.txt"

echo
echo "----------NEW BATCH-----------" 
echo "Starting point:    ${1}e${exp}"
echo "Thread chunk size: $chunk"
echo "Block size:        ${3}e${exp}"
echo "Stopping point:    ${2}e${exp}"

touch $tfile
echo >> $tfile
echo $(date "+%F %T") >> $tfile

tic=$(date +%s)
for (( i=$1; i<$2; i+=$3 )); do
	echo
	date "+%F %T"
	
	cmd="$x ${i}${zeros} ${3}${zeros} $chunk"
	echo "Running $cmd"
	total=$($cmd)

	echo "Moving output"
	next=$(($i+$3))
	from=$(printf "%06d" $i)
	to=$(printf "%06d" $next)
	
	name="${from}e${exp}-to-${to}e${exp}"
	outfile="$dir_storage/$name"

	mv "$dir_execute/output.txt" $outfile
	echo "Saved to $outfile"

	echo "${name}: $total" >> $tfile
	echo "Saved total to $tfile"
done
toc=$(date +%s)

echo
date "+%F %T"
echo "Finished! Total time: $(($toc-$tic)) seconds"
