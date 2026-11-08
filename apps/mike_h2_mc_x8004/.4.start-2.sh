#!/usr/bin/env bash

esp-emu \
  --chip esp32h2 \
  --firmware build/merged-binary.bin \
  --elf build/mike_h2_multiclimate.elf \
  --thread-sim "bind:9002,peer:127.0.0.1:9001" \
  --ble-hci tcp:localhost:9544 \
  --net "user,mdns-nat=yes"
