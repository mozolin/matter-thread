#!/usr/bin/env bash

# Create TAP interface with NAT (auto-detects outbound interface)
sudo tools/setup-tap.sh

esp-emu \
  --chip esp32h2 \
  --firmware saved/merged-binary.bin \
  --elf build/mike_h2_multiclimate.elf \
  --ble-hci tcp:localhost:9545 \
  --net "tap,ifname=tap0"
  #--net "user,mdns-nat=yes"

# Cleanup
sudo tools/setup-tap.sh --teardown
