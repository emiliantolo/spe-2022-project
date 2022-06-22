#!/bin/bash

for d in project_*
do
    printf "$d\n"
    chmod +x $d/run.sh
    (cd "$d" && ./run.sh --clean)
    printf "\n"
done
