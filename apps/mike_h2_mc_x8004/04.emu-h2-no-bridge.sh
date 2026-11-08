#!/usr/bin/env bash

# No bridge needed — device is already commissioned
esp-emu \
  --chip esp32h2 \
  --firmware saved/merged-binary.bin \
  --elf build/mike_h2_multiclimate.elf \
  --net "user,mdns-nat=yes"
