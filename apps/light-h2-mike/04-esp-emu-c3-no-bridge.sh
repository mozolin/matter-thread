#!/usr/bin/env bash

# No bridge needed — device is already commissioned
esp-emu \
  --chip esp32c3 \
  --firmware saved/merged-binary.bin \
  --elf build/light.elf \
  --net "user,mdns-nat=yes"
