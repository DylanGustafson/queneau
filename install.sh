#!/bin/bash

chmod +x qloop.sh qdaemon timer.sh
make queneau
make prime-sieve
./prime-sieve 15000000
echo Done!
