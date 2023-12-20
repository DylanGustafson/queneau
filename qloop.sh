#!/bin/bash
ch=1000000
zeros=$(printf "0$.0s" {1..9})
exp=${#zeros}

dir_execute="/Users/gustafsondy/queneau"
dir_storage="/UsersGbl/gustafsondy/queneau"
x="$dir_execute/queneau-list"
tfile="$dir_storage/totals.txt"

echo
echo "----------NEW BATCH-----------" 
echo "Starting point:    ${1}e${exp}"
echo "Thread chunk size: $ch"
echo "Block size:        ${3}e${exp}"
echo "Stopping point:    ${2}e${exp}"

touch $tfile
echo >> tfile
echo $(date "+%F %T") >> tfile

tic=$(date +%s)
for (( i=$1; i<$2; i+=$3 )); do
	echo
	date "+%F %T"
	echo "Running $x ${i}e${exp} ${3}e${exp} $ch"
	total=$($x ${i}${zeros} ${3}${zeros} $ch)
	echo "Moving output"

	next=$(echo "$i + $3" | bc)
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
echo "Finished! Total time: $(echo "$toc - $tic" | bc) seconds"
