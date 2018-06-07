#!/bin/bash

rm -f /dev/mqueue/clientqueue*

for ((i=0;i<$1;i++)); do
    ./client list$i.txt &
done

wait
