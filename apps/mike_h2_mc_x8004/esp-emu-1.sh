#!/usr/bin/env bash

#rm -rf ~/.esp-emu/

echo 'KERNEL=="vhci", MODE="0666"' | sudo tee /etc/udev/rules.d/99-vhci.rules
sudo udevadm trigger

sudo ip tuntap add dev tap0 mode tap user $USER
sudo ip addr add 192.168.4.1/24 dev tap0
sudo ip link set tap0 up

python3 tools/vhci_bridge.py

esp-emu --chip esp32h2 \
        --firmware build/merged-binary.bin \
        --elf build/mike_h2_multiclimate.elf \
        --net "user,mdns-nat=yes"
        --thread-sim "bind:9001,peer:127.0.0.1:9002"
