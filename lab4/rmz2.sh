#!/bin/bash

pids=`ps -aux | grep "$2" | cut -d" " -f3`

for pid in $pids; do
    echo "killed $pid"
    `kill -9 $pid`
done