#!/bin/bash

for d in project_campaign_diff/data/*/
do
    (cd "$d" && ./../../../count.sh > count.txt) > /dev/null 2>&1
done
