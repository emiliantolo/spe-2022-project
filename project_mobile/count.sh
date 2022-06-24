#!/bin/bash

for f in ./wifi-st*.pcap
do
    tcpdump -e -v -n -r $f | grep -c -i "retry.*DA:00:00:00:00:00:01"
done
