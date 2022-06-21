#!/bin/bash

for d in project_campaign_mobile/data/*/
do
    (cd "$d" && ./../../../count_mobile.sh > count.txt) > /dev/null 2>&1
done
