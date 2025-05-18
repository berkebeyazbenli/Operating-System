#!/bin/bash
rm -rf myDir
mkdir myDir
cd myDir
for i in $(seq 1 $((21 + RANDOM % 10))); do
  for j in $(seq 1 1000); do
    echo $RANDOM >> file$i.txt
  done
done
cd ..
