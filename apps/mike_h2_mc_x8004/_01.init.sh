#!/usr/bin/env bash

sudo brctl addbr br0
sudo tunctl -t tap0
sudo tunctl -t tap1
sudo brctl addif br0 tap0
sudo brctl addif br0 tap1
sudo ip link set br0 up
sudo ip link set tap0 up
sudo ip link set tap1 up

#-- Install chip-tool
sudo snap install chip-tool
sudo snap connect chip-tool:avahi-observe
sudo snap connect chip-tool:bluez

#-- Install Bumble
pip3 install bumble

#-- VHCI device permissions
sudo chmod 666 /dev/vhci

#-- For a persistent rule
echo 'KERNEL=="vhci", MODE="0666"' | sudo tee /etc/udev/rules.d/99-vhci.rules
sudo udevadm trigger

#-- Option A — user-mode networking with mDNS NAT (no host setup, recommended)
# esp-emu ... --net "user,mdns-nat=yes" ...

#-- Option B — TAP interface (full L2 bridge to the host network)
sudo ip tuntap add dev tap0 mode tap user $USER
sudo ip addr add 192.168.4.1/24 dev tap0
sudo ip link set tap0 up
# then run esp-emu with --net "tap,ifname=tap0"
sudo ip tuntap add dev tap0 mode tap user $USER
sudo ip addr add 192.168.4.2/24 dev tap1
sudo ip link set tap1 up
# then run esp-emu with --net "tap,ifname=tap1"



##-- Port already in use
pkill -f vhci_bridge
lsof -i :9544
##-- Start the VHCI bridge
python3 tools/vhci_bridge.py --port 9544 &
#
# [*] Opening /dev/vhci...
# [+] VHCI adapter registered as hci1
#     Use: chip-tool pairing ble-wifi ... --ble-controller 1
# [*] Starting TCP server on port 9544, waiting for esp-emu...
#
