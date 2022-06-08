tcpdump -e -v -n -r wifi-ap0-2-0.pcap | grep -c -i "retry.*proto udp"
tcpdump -e -v -n -r wifi-st0-0-0.pcap | grep -c -i "retry.*proto udp"
tcpdump -e -v -n -r wifi-st1-1-0.pcap | grep -c -i "retry.*proto udp"
