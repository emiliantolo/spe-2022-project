#!/bin/bash

python3 sem_simulations.py
python3 stats.py

if [[ $* == *--clean* ]]
then
    rm -r project_campaign
fi
