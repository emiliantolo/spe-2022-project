#!/bin/bash

python3 sem_simulations.py
chmod +x retry_count.sh
chmod +x count.sh
./retry_count.sh
python3 stats.py

if [[ $* == *--clean* ]]
then
    rm -r project_campaign
fi
