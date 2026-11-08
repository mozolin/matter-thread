#!/usr/bin/env bash

#esp-emu \
#  --chip esp32h2 \
#  --firmware build/merged-binary.bin \
#  --elf build/mike_h2_multiclimate.elf \
#  --thread-sim "bind:9001,peer:127.0.0.1:9002" \
#  --ble-hci tcp:localhost:9544 \
#  --net "user,mdns-nat=yes"


# Create TAP interface with NAT (auto-detects outbound interface)
sudo tools/setup-tap.sh

esp-emu \
  --chip esp32h2 \
  --firmware build/merged-binary.bin \
  --elf build/mike_h2_multiclimate.elf \
  --thread-sim "bind:9001,peer:127.0.0.1:9002" \
  #--ble-hci tcp:localhost:9544 \
  #--net "tap,ifname=tap0"
  --net "user,mdns-nat=yes"

# Cleanup
sudo tools/setup-tap.sh --teardown
