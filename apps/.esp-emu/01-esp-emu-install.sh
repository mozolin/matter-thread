#!/usr/bin/env bash

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
